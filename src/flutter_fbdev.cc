#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>

#include "embedder.h"
#include <istream>

#include <linux/fb.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

int fbfd;
int pixel_format;
fb_var_screeninfo vinfo;

fb_var_screeninfo GetScreenInfo(const int fbfd)
{
    struct fb_var_screeninfo vinfo;
    ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo); // todo check return value

    return vinfo;
}

void WriteFramebuffer(const void *framebuffer)
{   
    size_t screensize = vinfo.yres_virtual * vinfo.xres_virtual * vinfo.bits_per_pixel / 8;
    lseek(fbfd, 0, SEEK_SET);
    write(fbfd, framebuffer, screensize);
}

void PrintUsage()
{
    cout << "Usage: flutter_fbdev <project_path> <icudtl_path> <fb_device>" << endl;
}

bool RunFlutter(
    const string &project_path,
    const string &icudtl_path,
    const int fbfd)
{

    vinfo = GetScreenInfo(fbfd);

    if (vinfo.xres_virtual == 0 || vinfo.yres_virtual == 0)
    {
        cout << "Could not get screen info." << endl;
        return false;
    }

    cout << "Screen resolution: " << vinfo.xres_virtual << "x" << vinfo.yres_virtual << endl;

    FlutterRendererConfig config = {};
    config.type = kSoftware;
    config.software.struct_size = sizeof(config.software);
    config.software.surface_present_callback = [](void *user_data, const void *allocation, size_t row_bytes, size_t height) -> bool {
        WriteFramebuffer(allocation);
        return true;
    };

    string assets_path = project_path + "/build/flutter_assets";

    FlutterProjectArgs args = {};

    args.struct_size = sizeof(FlutterProjectArgs);
    args.assets_path = assets_path.c_str();
    args.icu_data_path = icudtl_path.c_str();
    args.compositor->create_backing_store_callback = [](void *user_data, FlutterBackingStore *store) -> bool {
        switch(pixel_format) {
            case 1: store->software2.pixel_format = kFlutterSoftwarePixelFormatGray8; break;
            case 2: store->software2.pixel_format = kFlutterSoftwarePixelFormatRGB565; break;
            case 3: store->software2.pixel_format = kFlutterSoftwarePixelFormatRGBA4444; break;
            case 4: store->software2.pixel_format = kFlutterSoftwarePixelFormatRGBA8888; break;
            case 5: store->software2.pixel_format = kFlutterSoftwarePixelFormatRGBX8888; break;
            case 6: store->software2.pixel_format = kFlutterSoftwarePixelFormatBGRA8888; break;
            case 7: store->software2.pixel_format = kFlutterSoftwarePixelFormatNative32; break;
            default: store->software2.pixel_format = kFlutterSoftwarePixelFormatRGBA8888; break;
        }

        return true;
    };
    
    FlutterEngine engine = nullptr;
    FlutterEngineResult result =
        FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config, &args, nullptr, &engine);

    if (result != kSuccess || engine == nullptr)
    {
        cout << "Could not run the Flutter Engine." << endl;
        return false;
    }
    FlutterEngineDisplay display = {};

    display.struct_size = sizeof(FlutterEngineDisplay);
    display.display_id = 0;
    display.single_display = true;
    display.refresh_rate = 60;


    vector<FlutterEngineDisplay> displays = {display};
    FlutterEngineNotifyDisplayUpdate(engine,
                                     kFlutterEngineDisplaysUpdateTypeStartup,
                                     displays.data(), displays.size());

    FlutterWindowMetricsEvent metrics_event = {};
    metrics_event.struct_size = sizeof(FlutterWindowMetricsEvent);
    metrics_event.width = vinfo.xres_virtual;
    metrics_event.height = vinfo.yres_virtual;
    metrics_event.pixel_ratio = 1.0;
    FlutterEngineSendWindowMetricsEvent(engine, &metrics_event);

    return true;
}

int main(int argc, const char *argv[])
{

    if (argc != 5)
    {
        PrintUsage();
        return 1;
    }

    string project_path = argv[1];
    string icudtl_path = argv[2];

    string fb_device = argv[3];
    pixel_format = atoi(argv[4]);


    fbfd = open(fb_device.c_str(), O_RDWR);
    if (fbfd == -1)
    {
        cout << "Could not open " << fb_device << endl;
        return 1;
    }

    RunFlutter(project_path, icudtl_path, fbfd);

    while (getchar() != 'q')
    {
    }

    close(fbfd);

    return 0;
}
