#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif

#include <Magick++.h>
#include <node.h>
#include <node_buffer.h>

using namespace v8;
using namespace Magick;

#include <list>
#include <string.h>
#include <exception>

#define THROW_ERROR_EXCEPTION(x) ThrowException(v8::Exception::Error(String::New(x))); \
scope.Close(Undefined())

// input
// args[ 0 ]: options. required, object with following key,values
// {
// src    : required. Buffer with binary image data
// format : optional. one of http://www.imagemagick.org/script/formats.php ex: "JPEG"
// debug  : optional. 1 or 0
// }
Handle<Value> ImgToPixmap(const Arguments& args) {
    HandleScope scope;
    MagickCore::SetMagickResourceLimit(MagickCore::ThreadResource, 1);

    if ( args.Length() != 1 ) {
        return THROW_ERROR_EXCEPTION("imgToPixmap() requires a single options hash as an argument");
    }
    if ( ! args[ 0 ]->IsObject() ) {
        return THROW_ERROR_EXCEPTION("imgToPixmap()'s argument should be an object");
    }
    Local<Object> obj = Local<Object>::Cast( args[ 0 ] );

    Local<Object> src = Local<Object>::Cast( obj->Get( String::NewSymbol("src") ) );
    
    bool noSrcGiven  = src->IsUndefined();
    bool srcIsString = !noSrcGiven && ( src->IsString() || src->IsStringObject() );
    bool srcIsBuffer = !noSrcGiven && node::Buffer::HasInstance(src);

    if (noSrcGiven || !(srcIsString || srcIsBuffer)) {
        return THROW_ERROR_EXCEPTION("imgToPixmap()'s argument should have \"src\" containing either a String filepath or a Buffer instance");
    }

    int debug = obj->Get( String::NewSymbol("debug") )->Uint32Value();
    if (debug) printf( "debug: on\n" );

    Magick::Image image;
    try {
      //TODO: This throws on libpng warning for iCCP profiles -- need to bypass
      if (srcIsString) {
        String::Utf8Value srcStr( src->ToString() );
        image.read( std::string(*srcStr) );
      } else {
        Magick::Blob srcBlob( node::Buffer::Data(src), node::Buffer::Length(src) );
        image.read( srcBlob );
      }
    }
    catch (std::exception& err) {
        std::string message = "image.read failed with error: ";
        message += err.what();
        return THROW_ERROR_EXCEPTION(message.c_str());
    }
    catch (...) {
        return THROW_ERROR_EXCEPTION("unhandled error");
    }

    int width  = (int) image.columns();
    int height = (int) image.rows();

    if (debug) printf("width, height: %d, %d\n", width, height);

    Local<Value> formatValue = obj->Get( String::NewSymbol("format") );
    String::AsciiValue format( formatValue->ToString() );
    if ( ! formatValue->IsUndefined() ) {
        if (debug) printf( "format: %s\n", *format );
        image.magick( *format );
    }

    const Magick::PixelPacket* pixels = image.getConstPixels(0, 0, width, height);

    unsigned int pixBuffLength = width * height * 4;

    node::Buffer* retBuffer = node::Buffer::New( pixBuffLength );
    unsigned char retPixels[pixBuffLength];

    for (int i = 0; i < width * height; i++) { 
        unsigned int r = (int) pixels[i].red;
        unsigned int g = (int) pixels[i].green;
        unsigned int b = (int) pixels[i].blue;
        unsigned int a = (int) pixels[i].opacity;

        // Convert RGB back onto 0-255 scale if needed
        retPixels[(i*4)]   = (255 * r) / MaxRGB;
        retPixels[(i*4)+1] = (255 * g) / MaxRGB;
        retPixels[(i*4)+2] = (255 * b) / MaxRGB;
        retPixels[(i*4)+3] = 255 - ((255 * a) / MaxRGB); // Magick inverts typical opacity scale
    }

    //return scope.Close( False() );
    memcpy( node::Buffer::Data( retBuffer ), retPixels, pixBuffLength );
    //return scope.Close( retBuffer->handle_ );

    //TODO: Should I bother converting SlowBuffer?
    //Local<Object> globalObj = Context::GetCurrent()->Global();
    //Local<Function> bufferConstructor = Local<Function>::Cast(globalObj->Get(String::New("Buffer")));
    //Handle<Value> constructorArgs[3] = { retBuffer->handle_, v8::Integer::New(pixBuffLength), v8::Integer::New(0) };
    //Local<Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);

    Local<Object> retObj = Object::New();
    retObj->Set(String::NewSymbol("width"), Number::New(width));
    retObj->Set(String::NewSymbol("height"), Number::New(height));
    //retObj->Set(String::NewSymbol("data"), actualBuffer);
    retObj->Set(String::NewSymbol("data"), retBuffer->handle_);

    return scope.Close(retObj);
}

void Init(Handle<Object> exports, Handle<Object> module) {
    module->Set(String::NewSymbol("exports"),
      FunctionTemplate::New(ImgToPixmap)->GetFunction());
}

NODE_MODULE(imgToPixmap, Init)