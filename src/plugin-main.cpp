#include <obs-module.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <cerrno>
#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

namespace {

constexpr const char *SETTING_BIND_ADDRESS = "bind_address";
constexpr const char *SETTING_PUBLIC_HOST = "public_host";
constexpr const char *SETTING_PORT = "port";
constexpr const char *SETTING_APP = "app_name";
constexpr const char *SETTING_STREAM_KEY = "stream_key";
constexpr const char *SETTING_FFMPEG_PATH = "ffmpeg_path";
constexpr const char *SETTING_AUTO_START = "auto_start";
constexpr const char *SETTING_UDP_PORT = "udp_port";
constexpr const char *SETTING_BUFFERING_MB = "buffering_mb";
constexpr const char *SETTING_FULL_URL = "full_stream_url";
constexpr const char *SETTING_SERVER_URL = "server_url";

static std::string obs_string(obs_data_t *settings, const char *name)
{
	const char *value = obs_data_get_string(settings, name);
	return value ? value : "";
}

static std::string sanitize_path_component(std::string value, const char *fallback)
{
	value.erase(std::remove_if(value.begin(), value.end(), [](unsigned char ch) {
		return !(std::isalnum(ch) || ch == '_' || ch == '-');
	}), value.end());
	return value.empty() ? fallback : value;
}

static std::string generate_stream_key()
{
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<unsigned long long> dist;
	std::ostringstream out;
	out << std::hex << std::setfill('0') << std::nouppercase;
	out << std::setw(16) << dist(gen) << std::setw(16) << dist(gen);
	return out.str();
}

static std::string make_server_url(const std::string &host, long long port, const std::string &app)
{
	std::ostringstream out;
	out << "rtmp://" << host << ':' << port << '/' << app;
	return out.str();
}

static std::string make_listen_url(const std::string &bind_address, long long port, const std::string &app,
				   const std::string &stream_key)
{
	std::ostringstream out;
	out << "rtmp://" << bind_address << ':' << port << '/' << app << '/' << stream_key;
	return out.str();
}

static std::string make_udp_url(long long port)
{
	std::ostringstream out;
	out << "udp://127.0.0.1:" << port << "?fifo_size=1000000&overrun_nonfatal=1";
	return out.str();
}

#ifdef _WIN32
static std::wstring widen(const std::string &value)
{
	if (value.empty())
		return L"";
	const int size = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
	std::wstring wide(size - 1, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, wide.data(), size);
	return wide;
}

static std::wstring quote_windows_arg(const std::string &arg)
{
	std::wstring wide = widen(arg);
	std::wstring quoted = L"\"";
	unsigned backslashes = 0;
	for (wchar_t ch : wide) {
		if (ch == L'\\') {
			backslashes++;
		} else if (ch == L'\"') {
			quoted.append(backslashes * 2 + 1, L'\\');
			quoted.push_back(ch);
			backslashes = 0;
		} else {
			quoted.append(backslashes, L'\\');
			backslashes = 0;
			quoted.push_back(ch);
		}
	}
	quoted.append(backslashes * 2, L'\\');
	quoted.push_back(L'\"');
	return quoted;
}
#endif

class ChildProcess {
public:
	bool start(const std::vector<std::string> &args)
	{
		stop();
		if (args.empty())
			return false;
#ifdef _WIN32
		std::wstring command;
		for (const auto &arg : args) {
			if (!command.empty())
				command.push_back(L' ');
			command += quote_windows_arg(arg);
		}
		STARTUPINFOW startup = {};
		PROCESS_INFORMATION process = {};
		startup.cb = sizeof(startup);
		const BOOL ok = CreateProcessW(nullptr, command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
					      nullptr, nullptr, &startup, &process);
		if (!ok)
			return false;
		CloseHandle(process.hThread);
		process_handle_ = process.hProcess;
		return true;
#else
		pid_t pid = fork();
		if (pid < 0)
			return false;
		if (pid == 0) {
			setpgid(0, 0);
			std::vector<char *> argv;
			argv.reserve(args.size() + 1);
			for (const auto &arg : args)
				argv.push_back(const_cast<char *>(arg.c_str()));
			argv.push_back(nullptr);
			execvp(argv[0], argv.data());
			_exit(127);
		}
		pid_ = pid;
		return true;
#endif
	}

	void stop()
	{
#ifdef _WIN32
		if (process_handle_) {
			TerminateProcess(process_handle_, 0);
			WaitForSingleObject(process_handle_, 3000);
			CloseHandle(process_handle_);
			process_handle_ = nullptr;
		}
#else
		if (pid_ > 0) {
			kill(-pid_, SIGTERM);
			for (int i = 0; i < 30; ++i) {
				if (waitpid(pid_, nullptr, WNOHANG) == pid_) {
					pid_ = -1;
					return;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			kill(-pid_, SIGKILL);
			waitpid(pid_, nullptr, 0);
			pid_ = -1;
		}
#endif
	}

	~ChildProcess() { stop(); }

private:
#ifdef _WIN32
	HANDLE process_handle_ = nullptr;
#else
	pid_t pid_ = -1;
#endif
};

struct RtmpReceiverSource {
	obs_source_t *source = nullptr;
	obs_source_t *media_source = nullptr;
	ChildProcess ffmpeg;
	std::string bind_address;
	std::string public_host;
	std::string app_name;
	std::string stream_key;
	std::string ffmpeg_path;
	long long rtmp_port = 1935;
	long long udp_port = 10000;
	long long buffering_mb = 2;
	bool auto_start = true;
	bool active = false;
};

static void create_or_update_media_source(RtmpReceiverSource *context)
{
	obs_data_t *media_settings = obs_data_create();
	obs_data_set_bool(media_settings, "local_file", false);
	obs_data_set_string(media_settings, "input", make_udp_url(context->udp_port).c_str());
	obs_data_set_string(media_settings, "input_format", "mpegts");
	obs_data_set_int(media_settings, "buffering_mb", context->buffering_mb);
	obs_data_set_bool(media_settings, "is_looping", false);
	obs_data_set_bool(media_settings, "restart_on_activate", false);
	obs_data_set_bool(media_settings, "close_when_inactive", false);
	obs_data_set_bool(media_settings, "clear_on_media_end", true);

	if (!context->media_source) {
		context->media_source = obs_source_create_private("ffmpeg_source", "RTMP receiver media", media_settings);
	} else {
		obs_source_update(context->media_source, media_settings);
	}
	obs_data_release(media_settings);
}

static void stop_receiver(RtmpReceiverSource *context)
{
	context->ffmpeg.stop();
}

static void start_receiver(RtmpReceiverSource *context)
{
	stop_receiver(context);
	create_or_update_media_source(context);

	const std::string listen_url = make_listen_url(context->bind_address, context->rtmp_port,
							 context->app_name, context->stream_key);
	const std::string output_url = make_udp_url(context->udp_port);
	std::vector<std::string> args = {
		context->ffmpeg_path,
		"-hide_banner",
		"-loglevel",
		"warning",
		"-listen",
		"1",
		"-i",
		listen_url,
		"-map",
		"0",
		"-c",
		"copy",
		"-f",
		"mpegts",
		output_url,
	};

	if (!context->ffmpeg.start(args)) {
		blog(LOG_ERROR, "RTMP Receiver: failed to launch FFmpeg at '%s'", context->ffmpeg_path.c_str());
	} else {
		blog(LOG_INFO, "RTMP Receiver listening on %s", listen_url.c_str());
	}
}

static void update_urls(obs_data_t *settings)
{
	std::string public_host = obs_string(settings, SETTING_PUBLIC_HOST);
	if (public_host.empty())
		public_host = "127.0.0.1";
	const long long port = obs_data_get_int(settings, SETTING_PORT) > 0 ? obs_data_get_int(settings, SETTING_PORT) : 1935;
	const std::string app = sanitize_path_component(obs_string(settings, SETTING_APP), "live");
	std::string key = obs_string(settings, SETTING_STREAM_KEY);
	if (key.empty())
		key = generate_stream_key();

	const std::string server = make_server_url(public_host, port, app);
	obs_data_set_string(settings, SETTING_APP, app.c_str());
	obs_data_set_string(settings, SETTING_STREAM_KEY, key.c_str());
	obs_data_set_string(settings, SETTING_SERVER_URL, server.c_str());
	obs_data_set_string(settings, SETTING_FULL_URL, (server + "/" + key).c_str());
}

static const char *rtmp_receiver_get_name(void *)
{
	return "RTMP Receiver (FFmpeg)";
}

static void rtmp_receiver_update(void *data, obs_data_t *settings)
{
	auto *context = static_cast<RtmpReceiverSource *>(data);
	update_urls(settings);
	context->bind_address = obs_string(settings, SETTING_BIND_ADDRESS);
	if (context->bind_address.empty())
		context->bind_address = "0.0.0.0";
	context->public_host = obs_string(settings, SETTING_PUBLIC_HOST);
	if (context->public_host.empty())
		context->public_host = "127.0.0.1";
	context->rtmp_port = obs_data_get_int(settings, SETTING_PORT);
	if (context->rtmp_port <= 0)
		context->rtmp_port = 1935;
	context->app_name = sanitize_path_component(obs_string(settings, SETTING_APP), "live");
	context->stream_key = obs_string(settings, SETTING_STREAM_KEY);
	if (context->stream_key.empty())
		context->stream_key = generate_stream_key();
	context->ffmpeg_path = obs_string(settings, SETTING_FFMPEG_PATH);
	if (context->ffmpeg_path.empty())
		context->ffmpeg_path = "ffmpeg";
	context->udp_port = obs_data_get_int(settings, SETTING_UDP_PORT);
	if (context->udp_port <= 0)
		context->udp_port = 10000;
	context->buffering_mb = obs_data_get_int(settings, SETTING_BUFFERING_MB);
	if (context->buffering_mb <= 0)
		context->buffering_mb = 2;
	context->auto_start = obs_data_get_bool(settings, SETTING_AUTO_START);

	create_or_update_media_source(context);
	if (context->active) {
		if (context->auto_start)
			start_receiver(context);
		else
			stop_receiver(context);
	}
}

static void *rtmp_receiver_create(obs_data_t *settings, obs_source_t *source)
{
	auto *context = new RtmpReceiverSource();
	context->source = source;
	rtmp_receiver_update(context, settings);
	return context;
}

static void rtmp_receiver_destroy(void *data)
{
	auto *context = static_cast<RtmpReceiverSource *>(data);
	stop_receiver(context);
	if (context->media_source)
		obs_source_release(context->media_source);
	delete context;
}

static void rtmp_receiver_activate(void *data)
{
	auto *context = static_cast<RtmpReceiverSource *>(data);
	context->active = true;
	if (context->auto_start)
		start_receiver(context);
}

static void rtmp_receiver_deactivate(void *data)
{
	auto *context = static_cast<RtmpReceiverSource *>(data);
	context->active = false;
	stop_receiver(context);
}

static uint32_t rtmp_receiver_get_width(void *data)
{
	auto *context = static_cast<RtmpReceiverSource *>(data);
	return context->media_source ? obs_source_get_width(context->media_source) : 0;
}

static uint32_t rtmp_receiver_get_height(void *data)
{
	auto *context = static_cast<RtmpReceiverSource *>(data);
	return context->media_source ? obs_source_get_height(context->media_source) : 0;
}

static void rtmp_receiver_video_render(void *data, gs_effect_t *)
{
	auto *context = static_cast<RtmpReceiverSource *>(data);
	if (context->media_source)
		obs_source_video_render(context->media_source);
}

static void rtmp_receiver_enum_active_sources(void *data, obs_source_enum_proc_t enum_callback, void *param)
{
	auto *context = static_cast<RtmpReceiverSource *>(data);
	if (context->media_source)
		enum_callback(context->source, context->media_source, param);
}

static bool regenerate_key_clicked(obs_properties_t *, obs_property_t *, void *data)
{
	auto *context = static_cast<RtmpReceiverSource *>(data);
	if (!context || !context->source)
		return false;

	obs_data_t *settings = obs_source_get_settings(context->source);
	obs_data_set_string(settings, SETTING_STREAM_KEY, generate_stream_key().c_str());
	update_urls(settings);
	obs_source_update(context->source, settings);
	obs_data_release(settings);
	return true;
}

static obs_properties_t *rtmp_receiver_properties(void *)
{
	obs_properties_t *props = obs_properties_create();
	obs_properties_add_text(props, SETTING_BIND_ADDRESS, "Bind address", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, SETTING_PUBLIC_HOST, "Public host / LAN IP to give streamers", OBS_TEXT_DEFAULT);
	obs_properties_add_int(props, SETTING_PORT, "RTMP port", 1, 65535, 1);
	obs_properties_add_text(props, SETTING_APP, "RTMP app/path", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, SETTING_STREAM_KEY, "Stream key", OBS_TEXT_PASSWORD);
	obs_properties_add_button(props, "regenerate_stream_key", "Generate new stream key", regenerate_key_clicked);
	obs_properties_add_text(props, SETTING_SERVER_URL, "Server URL to give streamer", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, SETTING_FULL_URL, "Full RTMP URL", OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, SETTING_FFMPEG_PATH, "FFmpeg executable", OBS_TEXT_DEFAULT);
	obs_properties_add_int(props, SETTING_UDP_PORT, "Local UDP relay port", 1, 65535, 1);
	obs_properties_add_int(props, SETTING_BUFFERING_MB, "OBS media buffer (MB)", 1, 64, 1);
	obs_properties_add_bool(props, SETTING_AUTO_START, "Start receiver when source is active");
	return props;
}

static void rtmp_receiver_defaults(obs_data_t *settings)
{
	obs_data_set_default_string(settings, SETTING_BIND_ADDRESS, "0.0.0.0");
	obs_data_set_default_string(settings, SETTING_PUBLIC_HOST, "127.0.0.1");
	obs_data_set_default_int(settings, SETTING_PORT, 1935);
	obs_data_set_default_string(settings, SETTING_APP, "live");
	obs_data_set_default_string(settings, SETTING_FFMPEG_PATH, "ffmpeg");
	obs_data_set_default_int(settings, SETTING_UDP_PORT, 10000);
	obs_data_set_default_int(settings, SETTING_BUFFERING_MB, 2);
	obs_data_set_default_bool(settings, SETTING_AUTO_START, true);
	update_urls(settings);
}

static obs_source_info rtmp_receiver_source_info = {};

} // namespace

bool obs_module_load(void)
{
	rtmp_receiver_source_info.id = "obs_rtmp_receiver_source";
	rtmp_receiver_source_info.type = OBS_SOURCE_TYPE_INPUT;
	rtmp_receiver_source_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_AUDIO;
	rtmp_receiver_source_info.get_name = rtmp_receiver_get_name;
	rtmp_receiver_source_info.create = rtmp_receiver_create;
	rtmp_receiver_source_info.destroy = rtmp_receiver_destroy;
	rtmp_receiver_source_info.get_width = rtmp_receiver_get_width;
	rtmp_receiver_source_info.get_height = rtmp_receiver_get_height;
	rtmp_receiver_source_info.get_defaults = rtmp_receiver_defaults;
	rtmp_receiver_source_info.get_properties = rtmp_receiver_properties;
	rtmp_receiver_source_info.update = rtmp_receiver_update;
	rtmp_receiver_source_info.activate = rtmp_receiver_activate;
	rtmp_receiver_source_info.deactivate = rtmp_receiver_deactivate;
	rtmp_receiver_source_info.video_render = rtmp_receiver_video_render;
	rtmp_receiver_source_info.enum_active_sources = rtmp_receiver_enum_active_sources;

	obs_register_source(&rtmp_receiver_source_info);
	blog(LOG_INFO, "%s %s loaded", PLUGIN_NAME, PLUGIN_VERSION);
	return true;
}
