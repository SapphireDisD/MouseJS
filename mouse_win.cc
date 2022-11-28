#include <windows.h>
#include <winuser.h>
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

void mouseInit(const FunctionCallbackInfo<Value> &args)
{
    return;
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

    SetCursorPos(x, y);

    return;
}

void mouseLeftClick(const FunctionCallbackInfo<Value> &args)
{
    POINT currentPosition;

    GetCursorPos(&currentPosition);

    INPUT inputs[3] = {};
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dx = currentPosition.x;
    inputs[0].mi.dy = currentPosition.y;

    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    inputs[2].type = INPUT_MOUSE;
    inputs[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));

    return;
}

void mouseRightClick(const FunctionCallbackInfo<Value> &args)
{
    POINT currentPosition;

    GetCursorPos(&currentPosition);

    INPUT inputs[3] = {};
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dx = currentPosition.x;
    inputs[0].mi.dy = currentPosition.y;

    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;

    inputs[2].type = INPUT_MOUSE;
    inputs[2].mi.dwFlags = MOUSEEVENTF_RIGHTUP;

    UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));

    return;
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
