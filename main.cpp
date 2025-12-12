#include "utils.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
int main() {

    // Image

    int image_width = 600;
    double aspect_ratio = 16.0 / 9.0;
    double viewport_height = 2.0;
    double focal_length = 1.0;

    hittable_list world;
    world.add(make_shared<sphere>(point3(0,0,-1), 0.5));
    world.add(make_shared<sphere>(point3(0,-100.5,-1), 100));   // green sphere that looks like ground
    // Render
    Camera mainCamera = Camera(image_width, aspect_ratio, viewport_height, focal_length, world);
    mainCamera.enableAA = true;
    mainCamera.reflectance_coeff = 0.7;
    mainCamera.Render();

}