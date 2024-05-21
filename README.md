*This project is not actively maintained.*

Nano SVG
==========

## Embedded
Added a usage example of rendering using file streams and piecewise rasterization for embedded rendering with limited memory.
With limited memory of 3MB RAM, it can parse and render SVG files of any size, such as 10 MB or more.

The following example uses OpenCV to display a svg image
![screenshot of some splines rendered with the sample program](/example/screenshot-3.jpg?raw=true)
```cpp
 // Load SVG
 NSVGimage *image;
 image = nsvgParseFromFile("test.svg", "px", 96);

 cv::Mat mat = cv::Mat::zeros(cv::Size(640, 480), CV_8UC4);

 Transform T;
 T.sx = mat.size().width / image->width;
 T.sy = mat.size().height / image->height;
 T.scale = T.sx < T.sy ? T.sx : T.sy;
 T.dx = (mat.size().width - image->width * T.scale) / 2.f;
 T.dy = (mat.size().height - image->height * T.scale) / 2.F;

// Use...
for (NSVGshape *shape = image->shapes; shape != NULL; shape = shape->next) {
  for (NSVGpath *path = shape->paths; path != NULL; path = path->next) {
    for (int i = 0; i < path->npts - 1; i += 3) {
        float *p = &path->pts[i * 2];
        CubicBezierCurve curve = Bezier::fromPoints({p[0], p[1]}, {p[2], p[3]},
                                                    {p[4], p[5]}, {p[6], p[7]});
        PolygonF polygon = Bezier::toPolygon(curve);
        for (auto i = 1; i < polygon.size(); i++) {
            // Draw Line
            // polygon[i-1],polygon[i]
            auto p1 = polygon[i - 1];
            auto p2 = polygon[i];
            cv::line(
                    mat,
                    cv::Point{int(p1.x * T.scale + T.dx), int(p1.y * T.scale + T.dy)},
                    cv::Point{int(p2.x * T.scale + T.dx), int(p2.y * T.scale + T.dy)},
                    cv::Scalar(0, 255, 0), 1);
        }
    } // end CubicBezierCurve
  }
}

// Delete
nsvgDelete(image);

cv::imshow("mat", mat);
cv::waitKeyEx();
```

Set the debug startup environment variable

launch.vs.json
```
{
  "version": "0.2.1",
  "defaults": {},
  "configurations": [
    {
      "type": "default",
      "project": "CMakeLists.txt",
      "projectTarget": "nanoembedded.exe",
      "name": "nanoembedded.exe",
      "env": {
        "PATH": "PATH=${env.PATH};C:\\Hub\\Source\\opencv\\build\\x64\\vc16\\bin;"
      }
    }
  ]
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
