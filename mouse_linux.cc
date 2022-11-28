#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <linux/uinput.h>
#include <node.h>

using namespace v8;

class AddonData
{
public:
    explicit AddonData(Isolate *isolate) : call_count(0)
    {
        node::AddEnvironmentCleanupHook(isolate, DeleteInstance, this);
    }

    int call_count;

    static void DeleteInstance(void *data)
    {
        delete static_cast<AddonData *>(data);
    }
};

int fd;

void emit(int fd, int type, int code, int val)
{
    struct input_event ie;

    ie.type = type;
    ie.code = code;
    ie.value = val;

    /* timestamp values below are ignored */
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    write(fd, &ie, sizeof(ie));
}

void mouseInit()
{
    struct uinput_setup usetup;

    fd = open("/dev/input/event0", O_WRONLY | O_NONBLOCK);

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);

    ioctl(fd, UI_SET_EVBIT, EV_ABS);
    ioctl(fd, UI_SET_ABSBIT, ABS_X);
    ioctl(fd, UI_SET_ABSBIT, ABS_Y);

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x5678;

    strcpy(usetup.name, "MouseJS");

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    sleep(1);
}

void mouseMove(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();

    if (args.Length() < 2)
    {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "What's the X and Y?").ToLocalChecked()));
        return;
    }

    if (!args[0]->IsNumber() || !args[1]->IsNumber())
    {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "X and Y need to be numbers").ToLocalChecked()));
        return;
    }

    double x = args[0].As<Number>()->Value();
    double y = args[1].As<Number>()->Value();
    emit(fd, EV_ABS, ABS_X, x);
    emit(fd, EV_ABS, ABS_Y, y);

    emit(fd, EV_SYN, SYN_REPORT, 0);
}

void mouseLeftClick()
{
    emit(fd, EV_KEY, BTN_LEFT, 1);
    emit(fd, EV_SYN, SYN_REPORT, 0);

    emit(fd, EV_KEY, BTN_LEFT, 0);
    emit(fd, EV_SYN, SYN_REPORT, 0);
}

void mouseRightClick()
{
    emit(fd, EV_KEY, BTN_RIGHT, 1);
    emit(fd, EV_SYN, SYN_REPORT, 0);

    emit(fd, EV_KEY, BTN_RIGHT, 0);
    emit(fd, EV_SYN, SYN_REPORT, 0);
}

NODE_MODULE_INIT()
{
    Isolate *isolate = context->GetIsolate();
    AddonData *data = new AddonData(isolate);
    Local<External> external = External::New(isolate, data);

    exports->Set(context,
                 String::NewFromUtf8(isolate, "init").ToLocalChecked(),
                 FunctionTemplate::New(isolate, mouseInit, external)
                     ->GetFunction(context)
                     .ToLocalChecked())
        .FromJust();

    exports->Set(context,
                 String::NewFromUtf8(isolate, "move").ToLocalChecked(),
                 FunctionTemplate::New(isolate, mouseMove, external)
                     ->GetFunction(context)
                     .ToLocalChecked())
        .FromJust();

    exports->Set(context,
                 String::NewFromUtf8(isolate, "leftClick").ToLocalChecked(),
                 FunctionTemplate::New(isolate, mouseLeftClick, external)
                     ->GetFunction(context)
                     .ToLocalChecked())
        .FromJust();

    exports->Set(context,
                 String::NewFromUtf8(isolate, "rightClick").ToLocalChecked(),
                 FunctionTemplate::New(isolate, mouseRightClick, external)
                     ->GetFunction(context)
                     .ToLocalChecked())
        .FromJust();
}
