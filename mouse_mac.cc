#import <CoreGraphics/CoreGraphics.h>
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

    CGDirectDisplayID displayId = CGMainDisplayID();
    CGPoint point = CGPointMake(x, y);
    CGDisplayMoveCursorToPoint(displayId, point);

    return;
}

void mouseLeftClick(const FunctionCallbackInfo<Value> &args)
{
    CGEventRef magicEvent = CGEventCreate(NULL);
    CGPoint currentPosition = CGEventGetLocation(magicEvent);
    CFRelease(magicEvent);

    CGEventRef mouseDown = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseDown, currentPosition, kCGMouseButtonLeft);
    CGEventPost(kCGHIDEventTap, mouseDown);

    CGEventRef mouseUp = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseUp, currentPosition, kCGMouseButtonLeft);
    CGEventPost(kCGHIDEventTap, mouseUp);

    return;
}

void mouseRightClick(const FunctionCallbackInfo<Value> &args)
{
    CGEventRef magicEvent = CGEventCreate(NULL);
    CGPoint currentPosition = CGEventGetLocation(magicEvent);
    CFRelease(magicEvent);

    CGEventRef mouseDown = CGEventCreateMouseEvent(NULL, kCGEventRightMouseDown, currentPosition, kCGMouseButtonRight);
    CGEventPost(kCGHIDEventTap, mouseDown);

    CGEventRef mouseUp = CGEventCreateMouseEvent(NULL, kCGEventRightMouseUp, currentPosition, kCGMouseButtonRight);
    CGEventPost(kCGHIDEventTap, mouseUp);

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
