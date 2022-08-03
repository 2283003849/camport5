#include <stdio.h>
#include "../common/common.hpp"
using namespace std;

#if 1
struct CamInfo
{
    char                sn[32];
    TY_INTERFACE_HANDLE hIface;
    TY_DEV_HANDLE       hDev;
    std::vector<char>   fb[2];
    TY_FRAME_DATA       frame;
    int                 idx;
    DepthRender         render;
    CamInfo() : hDev(0), idx(0) {}
};
int g_nSaveIndex = 0;
int g_nLoopCount = 0;

void del(char str[], char c)
{
    int j = 0;
    for (int i = 0; str[i] != '\0'; i++)
        if (str[i] != c)
            str[j++] = str[i];//将字符串中与要删除字符不同的字符保留，相同字符则跳过
    //str[j] = '\0';//在字符串末尾加上'\0'
}

char *getPicPath() {
    char c[1000];
    FILE* fptr = fopen("C:/camera/config/setting.txt", "r");
    char* ptr;
    if (fgets(c, sizeof(c), fptr) != NULL)
    {
        //printf("%s", c);
        ptr = strchr(c, '=');
        //printf("%s\n", ptr);
        del(ptr, '=');
        //printf("%s\n", ptr);
        
        //printf("aaa");
        //strncpy(c, c2, 10);
    }

    //string str = string(ptr);

    //printf("%c", ptr);
    fclose(fptr);
    return ptr;
}


string read_data(string path)
{
    ifstream ifs;					//创建流对象

    ifs.open(path, ios::in);		//以读的方式打开文件

    if (!ifs.is_open())				//判断文件是否打开
        return "打开失败!";

    for (int i = 0; i < 6; i++)
    {
        string str;

        if (i == 0)					// i = 5 ，是想要获取的行，可根据需要自行设定
        {
            getline(ifs, str, '\n');
            
            return str;
        }
        getline(ifs, str, '\n');    // 读取当前行内容，存入存str中 
    }
    ifs.close();

    return "憨憨！";
}


string getDeepImageSavePath() {

    
    g_nSaveIndex = g_nSaveIndex + 1;
    if (g_nSaveIndex > 10000) {
        g_nSaveIndex = 0;
    }


    string sPathPartEnd = ".jpg";
    string sSaveFramePath = "R:/Temp/" + to_string(g_nSaveIndex) + sPathPartEnd;
    //string sSaveFramePath = "F:\Temp\\" + to_string(g_nSaveIndex) + sPathPartEnd;
    return sSaveFramePath;
}

string getDeepSavePath() {
    g_nSaveIndex = g_nSaveIndex + 1;
    if (g_nSaveIndex > 10000) {
        g_nSaveIndex = 0;
    }

    
    string sPathPartEnd = ".depth";
    string sSaveFramePath = "R:/Temp/" + to_string(g_nSaveIndex) + sPathPartEnd;
    /*char *path = getPicPath();
    
    printf("%s", path);*/
    
    //string data = read_data("C:/camera/config/setting.txt");

    
    //string sSaveFramePath = "F:\Temp\\" + to_string(g_nSaveIndex) + sPathPartEnd;
    //printf("%s", getPicPath());
    return sSaveFramePath;

}

void frameHandler(TY_FRAME_DATA* frame, void* userdata)
{
    CamInfo* pData = (CamInfo*)userdata;
    cv::Mat depth, irl, irr, color;
    parseFrame(*frame, &depth, &irl, &irr, &color);
    g_nLoopCount++;
    char win[64];
    if (!depth.empty()) {
        cv::Mat colorDepth = pData->render.Compute(depth);
        sprintf(win, "depth-%s", pData->sn);
        cv::imshow(win, colorDepth);
        if (g_nLoopCount % 2 == 0) {
        cv:imwrite(getDeepImageSavePath(), colorDepth);
            FILE* objDeepFile = fopen(getDeepSavePath().c_str(), "wb");
            uint16_t lsDeep[480 * 640];
            for (int i = 0; i < 640; i++)
            {
                for (int j = 0; j < 480; j++)
                {
                    lsDeep[i * 480 + j] = depth.at<uint16_t>(j, i);
                }
            }
            fwrite(lsDeep, sizeof(uint16_t), 480 * 640, objDeepFile);
            fclose(objDeepFile);
            printf("%d=  ,", depth.at<uint16_t>(290, 346));

        }
    }
    if (!irl.empty()) {
        sprintf(win, "LeftIR-%s", pData->sn);
        cv::imshow(win, irl);
    }
    if (!irr.empty()) {
        sprintf(win, "RightIR-%s", pData->sn);
        cv::imshow(win, irr);
    }
    if (!color.empty()) {
        sprintf(win, "color-%s", pData->sn);
        cv::imshow(win, color);
    }

    pData->idx++;

    //LOGD("=== Re-enqueue buffer(%p, %d)", frame->userBuffer, frame->bufferSize);
    ASSERT_OK(TYEnqueueBuffer(pData->hDev, frame->userBuffer, frame->bufferSize));
}

void eventCallback(TY_EVENT_INFO* event_info, void* userdata)
{
    if (event_info->eventId == TY_EVENT_DEVICE_OFFLINE) {
        LOGD("=== Event Callback: Device Offline!");

        // Note: f
        //     Please set TY_BOOL_KEEP_ALIVE_ONOFF feature to false if you need to debug with breakpoint!
    }
    else if (event_info->eventId == TY_EVENT_LICENSE_ERROR) {
        LOGD("=== Event Callback: License Error!");
    }
}

int main(int argc, char* argv[])
{
    int32_t deviceType = TY_INTERFACE_ETHERNET | TY_INTERFACE_USB;

    std::vector<char*> list;
    int32_t cam_size = 0;
    int32_t found;
    bool    trigger_mode = false;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            LOGI("Usage: %s [-h]", argv[0]);
            return 0;
        }
        else if (strcmp(argv[i], "-t") == 0) {
            if (argc == i + 1) {
                LOGI("===== no type input");
                return 0;
            }
            else if (strcmp(argv[i + 1], "usb") == 0) {
                LOGI("===== Select usb device");
                deviceType = TY_INTERFACE_USB;
            }
            else if (strcmp(argv[i + 1], "net") == 0) {
                LOGI("===== Select ethernet device");
                deviceType = TY_INTERFACE_ETHERNET;
            }
        }
        else if (strcmp(argv[i], "-list") == 0) {
            if (argc == i + 1) {
                LOGI("===== no list input");
                return 0;
            }
            int32_t j;
            for (j = 0; j < argc - (i + 1); j++) {
                list.push_back(argv[i + 1 + j]);
                LOGI("===== Select device %s", list[j]);
            }
        }
        else if (strcmp(argv[i], "-trigger") == 0) {
            trigger_mode = true;
        }
    }

    //printf("deviceType: %d\n", deviceType);

    LOGD("=== Init lib");
    ASSERT_OK(TYInitLib());
    TY_VERSION_INFO ver;
    ASSERT_OK(TYLibVersion(&ver));
    LOGD("     - lib version: %d.%d.%d", ver.major, ver.minor, ver.patch);

    std::vector<TY_DEVICE_BASE_INFO> selected;
    ASSERT_OK(selectDevice(deviceType, "", "", 100, selected));

    //printf("size: %d\n", list.size()); 
    if (list.size()) {
        cam_size = list.size();
    }
    else {
        cam_size = selected.size();
    }

    std::vector<CamInfo> cams(cam_size);
    int32_t count = 0;
    for (uint32_t i = 0; i < selected.size(); i++) {
        if (list.size()) {
            found = 0;
            for (uint32_t j = 0; j < list.size(); j++) {
                LOGD("=== check device: %s, %s", selected[i].id, list[j]);
                if (strcmp(selected[i].id, list[j]) == 0) {
                    LOGD("=== check device success");
                    found = 1;
                    continue;
                }
            }

            if (!found) {
                continue;
            }
        }

        LOGD("=== Open device: %s", selected[i].id);
        strncpy(cams[count].sn, selected[i].id, sizeof(cams[count].sn));
        ASSERT_OK(TYOpenInterface(selected[i].iface.id, &cams[count].hIface));
        ASSERT_OK(TYOpenDevice(cams[count].hIface, selected[i].id, &cams[count].hDev));

        LOGD("=== Configure components, open depth cam");
        int componentIDs = TY_COMPONENT_DEPTH_CAM;
        ASSERT_OK(TYEnableComponents(cams[count].hDev, componentIDs));

        //try to enable depth map
        LOGD("Configure components, open depth cam");
        if (componentIDs & TY_COMPONENT_DEPTH_CAM) {
            int32_t image_mode;
            ASSERT_OK(get_default_image_mode(cams[count].hDev, TY_COMPONENT_DEPTH_CAM, image_mode));
            LOGD("Select Depth Image Mode: %dx%d", TYImageWidth(image_mode), TYImageHeight(image_mode));
            ASSERT_OK(TYSetEnum(cams[count].hDev, TY_COMPONENT_DEPTH_CAM, TY_ENUM_IMAGE_MODE, image_mode));
            ASSERT_OK(TYEnableComponents(cams[count].hDev, TY_COMPONENT_DEPTH_CAM));
        }

        LOGD("=== Prepare image buffer");
        uint32_t frameSize;
        ASSERT_OK(TYGetFrameBufferSize(cams[count].hDev, &frameSize));
        LOGD("     - Get size of framebuffer, %d", frameSize);

        LOGD("     - Allocate & enqueue buffers");
        cams[count].fb[0].resize(frameSize);
        cams[count].fb[1].resize(frameSize);
        LOGD("     - Enqueue buffer (%p, %d)", cams[count].fb[0].data(), frameSize);
        ASSERT_OK(TYEnqueueBuffer(cams[count].hDev, cams[count].fb[0].data(), frameSize));
        LOGD("     - Enqueue buffer (%p, %d)", cams[count].fb[1].data(), frameSize);
        ASSERT_OK(TYEnqueueBuffer(cams[count].hDev, cams[count].fb[1].data(), frameSize));

        LOGD("=== Register event callback");
        ASSERT_OK(TYRegisterEventCallback(cams[count].hDev, eventCallback, NULL));

        LOGD("=== Disable trigger mode");
        TY_TRIGGER_PARAM trigger;
        if (trigger_mode) {
            bool hasResend;
            ASSERT_OK(TYHasFeature(cams[count].hDev, TY_COMPONENT_DEVICE, TY_BOOL_GVSP_RESEND, &hasResend));
            if (hasResend) {
                LOGD("=== Open resend");
                ASSERT_OK(TYSetBool(cams[count].hDev, TY_COMPONENT_DEVICE, TY_BOOL_GVSP_RESEND, true));
            }
            trigger.mode = TY_TRIGGER_MODE_SLAVE;
        }
        else {
            trigger.mode = TY_TRIGGER_MODE_OFF;
        }
        ASSERT_OK(TYSetStruct(cams[count].hDev, TY_COMPONENT_DEVICE, TY_STRUCT_TRIGGER_PARAM, &trigger, sizeof(trigger)));

        LOGD("=== Start capture");
        ASSERT_OK(TYStartCapture(cams[count].hDev));
        count++;
    }

    if (count != cam_size) {
        LOGD("Invalid ids in input id list");
        return 0;
    }

    LOGD("=== While loop to fetch frame");
    bool exit_main = false;
    int cam_index = 0;
    int triggered = 0;
    while (!exit_main) {
        if (trigger_mode && triggered == 0) {
            for (size_t i = 0; i < cams.size(); i++) {
                ASSERT_OK(TYSendSoftTrigger(cams[i].hDev));
            }
            triggered = cams.size();
            LOGD("triggered once");
        }

        int err = TYFetchFrame(cams[cam_index].hDev, &cams[cam_index].frame, 10000);
        if (err != TY_STATUS_OK) {
            // LOGD("cam %s %d ... Drop one frame", cams[cam_index].sn, cams[cam_index].idx);
        }
        else {
            // LOGD("cam %s %d got one frame", cams[cam_index].sn, cams[cam_index].idx);
            triggered--;

            frameHandler(&cams[cam_index].frame, &cams[cam_index]);

            int key = cv::waitKey(1);
            switch (key & 0xff) {
            case 0xff:
                break;
            case 'q':
                exit_main = true;
                break;
            default:
                LOGD("Unmapped key %d", key);
            }
        }
        cam_index = (cam_index + 1) % cams.size();
    }

    for (uint32_t i = 0; i < cams.size(); i++) {
        ASSERT_OK(TYStopCapture(cams[i].hDev));
        ASSERT_OK(TYCloseDevice(cams[i].hDev));
        ASSERT_OK(TYCloseInterface(cams[i].hIface));
    }
    ASSERT_OK(TYDeinitLib());

    LOGD("=== Main done!");
    return 0;
}
#endif
