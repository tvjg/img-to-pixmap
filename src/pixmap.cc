#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif

#include <iostream>
#include <vector>
#include <algorithm>
#include <exception>

#include <Magick++.h>
#include <node.h>
#include <node_buffer.h>

using namespace v8;
using namespace Magick;

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
      if (srcIsString) {
        String::Utf8Value srcStr( src->ToString() );
        image.read( std::string(*srcStr) );
      } else {
        Magick::Blob srcBlob(node::Buffer::Data(src), node::Buffer::Length(src));
        image.read(srcBlob);
      }
    }
    catch (Magick::Warning& err) {
        std::cerr << "Warning: " << err.what() << "\n";
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

    size_t pixBuffLength = width * height * 4;

    std::vector<unsigned char> retPixels(pixBuffLength);
    node::Buffer* retBuffer = node::Buffer::New( pixBuffLength );

    for (size_t i = 0; i < width * height; i++) { 
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

    std::copy(retPixels.begin(), retPixels.end(), node::Buffer::Data(retBuffer));

    Local<Object> retObj = Object::New();
    retObj->Set(String::NewSymbol("width"), Number::New(width));
    retObj->Set(String::NewSymbol("height"), Number::New(height));
    retObj->Set(String::NewSymbol("data"), retBuffer->handle_);

    return scope.Close(retObj);
}

void Init(Handle<Object> exports, Handle<Object> module) {
    InitializeMagick(NULL);

    module->Set(String::NewSymbol("exports"),
      FunctionTemplate::New(ImgToPixmap)->GetFunction());
}

NODE_MODULE(imgToPixmap, Init)
