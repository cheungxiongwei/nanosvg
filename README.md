*This project is not actively maintained.*

Nano SVG
==========

## Embedded
Added a usage example of rendering using file streams and piecewise rasterization for embedded rendering with limited memory.
With limited memory of 3MB RAM, it can parse and render SVG files of any size, such as 10 MB or more.

The following example uses OpenCV to display a rasterized image
```cpp
#include <print>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

#include "nanosparser.h"

using namespace std;
using namespace cv::utils::logging;

template<typename T>
struct bbox_t {
    T x, y, width, height;
};

using f_bbox_t = bbox_t<float>;

typedef struct {
    float width;
    float height;
} preview_t;

cv::Mat loadFromFile(const std::string &filename, preview_t preview) {
    SVGParser parser;
    parser.open(filename.c_str(), "px", 96);
    parser.parser(); // [1]

    if(!parser.isValid()) {
        parser.close();
        return {};
    }

    parser.repaser(); // [2]

    auto size = parser.size();

    auto bbox = parser.bounds();

    uint8_t *buffer            = NULL;
    int clip_width  = 0;
    int clip_height = 0;

    // Calculate scaling & rendering & crop true content size
    auto source = preview_t {.width = size.width, .height = size.height};
    float scale  = SVGParser::CalcRasterizerScale(source.width, source.height, preview.width, preview.height);

    int width  = (int)(size.width * scale);
    int height = (int)(size.height * scale);

    cv::Mat mat = cv::Mat::zeros(cv::Size(width, height), CV_8UC4);

    float tx {0}, ty {0};
    if(bbox.x > 0) {
        tx = -bbox.x * scale;
    }
    if(bbox.y > 0) {
        ty = -bbox.y * scale;
    }

    parser.render(tx, ty, scale, mat.data, width, height); // [3]

    clip_width  = bbox.width * scale;
    clip_height = bbox.height * scale;

    if(clip_width > width) {
        clip_width = width;
    }

    if(clip_height > height) {
        clip_height = height;
    }

    // TODO clip

    parser.close();

    return mat.clone();
}

int main() {
    setLogLevel(LogLevel::LOG_LEVEL_WARNING);

    // auto svg = R"(example\zoom-out.svg)";
    auto svg = R"(example\tig.svg)";

    cv::TickMeter cost;
    cost.start();
    auto mat = loadFromFile(svg, {480, 320});
  
    cost.stop();
    std::println("cost:{}",cost.getTimeMilli());

    if(!mat.empty()) {
        std::vector<cv::Mat> a;
        cv::split(mat, a);

        cv::Mat binary = ~a[3];
        if(!cv::hasNonZero(binary)) {
            binary = a[2];
        } 
       
        cv::namedWindow("windows");
        cv::imshow("windows", binary);
    } else {
        mat = cv::Mat::zeros({480, 320}, CV_8UC3);
        cv::namedWindow("windows");
        cv::imshow("windows", mat);
    }

    while(true) {
        if(cv::waitKey(10) == 'q') break;
    }

    return 0;
}
```

## Parser

![screenshot of some splines rendered with the sample program](/example/screenshot-1.png?raw=true)

NanoSVG is a simple stupid single-header-file SVG parse. The output of the parser is a list of cubic bezier shapes.

The library suits well for anything from rendering scalable icons in your editor application to prototyping a game.

NanoSVG supports a wide range of SVG features, but something may be missing, feel free to create a pull request!

The shapes in the SVG images are transformed by the viewBox and converted to specified units.
That is, you should get the same looking data as your designed in your favorite app.

NanoSVG can return the paths in few different units. For example if you want to render an image, you may choose
to get the paths in pixels, or if you are feeding the data into a CNC-cutter, you may want to use millimeters. 

The units passed to NanoSVG should be one of: 'px', 'pt', 'pc' 'mm', 'cm', or 'in'.
DPI (dots-per-inch) controls how the unit conversion is done.

If you don't know or care about the units stuff, "px" and 96 should get you going.

## Rasterizer

![screenshot of tiger.svg rendered with NanoSVG rasterizer](/example/screenshot-2.png?raw=true)

The parser library is accompanied with really simpler SVG rasterizer. Currently it only renders flat filled shapes.

The intended usage for the rasterizer is to for example bake icons of different size into a texture. The rasterizer is not particular fast or accurate, but it's small and packed in one header file.


## Example Usage

``` C
// Load
struct NSVGimage* image;
image = nsvgParseFromFile("test.svg", "px", 96);
printf("size: %f x %f\n", image->width, image->height);
// Use...
for (shape = image->shapes; shape != NULL; shape = shape->next) {
	for (path = shape->paths; path != NULL; path = path->next) {
		for (i = 0; i < path->npts-1; i += 3) {
			float* p = &path->pts[i*2];
			drawCubicBez(p[0],p[1], p[2],p[3], p[4],p[5], p[6],p[7]);
		}
	}
}
// Delete
nsvgDelete(image);
```

## Using NanoSVG in your project

In order to use NanoSVG in your own project, just copy nanosvg.h to your project.
In one C/C++ define `NANOSVG_IMPLEMENTATION` before including the library to expand the NanoSVG implementation in that file.
NanoSVG depends on `stdio.h` ,`string.h` and `math.h`, they should be included where the implementation is expanded before including NanoSVG. 

``` C
#include <stdio.h>
#include <string.h>
#include <math.h>
#define NANOSVG_IMPLEMENTATION	// Expands implementation
#include "nanosvg.h"
```

By default, NanoSVG parses only the most common colors. In order to get support for full list of [SVG color keywords](http://www.w3.org/TR/SVG11/types.html#ColorKeywords), define `NANOSVG_ALL_COLOR_KEYWORDS` before expanding the implementation.

``` C
#include <stdio.h>
#include <string.h>
#include <math.h>
#define NANOSVG_ALL_COLOR_KEYWORDS	// Include full list of color keywords.
#define NANOSVG_IMPLEMENTATION		// Expands implementation
#include "nanosvg.h"
```

Alternatively, you can install the library using CMake and import it into your project using the standard CMake `find_package` command.

```CMake
add_executable(myexe main.c)

find_package(NanoSVG REQUIRED)

target_link_libraries(myexe NanoSVG::nanosvg NanoSVG::nanosvgrast)
```

## Compiling Example Project

In order to compile the demo project, your will need to install [GLFW](http://www.glfw.org/) to compile.

NanoSVG demo project uses [premake4](http://industriousone.com/premake) to build platform specific projects, now is good time to install it if you don't have it already. To build the example, navigate into the root folder in your favorite terminal, then:

- *OS X*: `premake4 xcode4`
- *Windows*: `premake4 vs2010`
- *Linux*: `premake4 gmake`

See premake4 documentation for full list of supported build file types. The projects will be created in `build` folder. An example of building and running the example on OS X:

```bash
$ premake4 gmake
$ cd build/
$ make
$ ./example
```

# License

The library is licensed under [zlib license](LICENSE.txt)
