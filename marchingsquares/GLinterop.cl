__kernel void texture_kernel(
  __write_only image2d_t im,
  int w, int h,
  int max_iter)
{
  int2 coord = { get_global_id(0), get_global_id(1) };

  // ideiglenes kód:
  /*float4 color = {
    coord.x / (float)w, // R
    coord.y / (float)h,	// G
    1.0f, 				// B
    1.0f };				// A

  write_imagef(im, coord, color);*/

  // Task 1: Draw a red disk! (i.e. inner points of a circle)
  /*float4 color = {1.0f, 0.0f, 0.0f, 1.0f};
  float2 c = {coord.x / (float)w - 0.5f, coord.y / (float)h - 0.5f }; // -0.5..0.5, -0.5..0.5
  float r = 0.25;

  if (dot(c,c) > r*r) {
    color.y = 1.0f;
    color.z = 1.0f;
  }
  write_imagef(im, coord, color);*/

  // Task 2: Visualize the Mandelbrot set! (see help below)
  float2 c = { coord.x / (float)w - 0.5f, coord.y / (float)h - 0.5f }; // -0.5..0.5, -0.5..0.5

  // zoom + movement
  c *= 3.0f;
  c.x -= 0.5f;

  float2 z = c;
  
  int iter = 0;
  for (; iter < max_iter; ++iter)
  {
    // z = z^2 + c
    float3 tmp = z.xyx * z.xyy; // SWIZZLE
    z.x = tmp.x - tmp.y; // Re
    z.y = 2*tmp.z; // Im
    z += c;

    // TODO: sufficient condition for divergence
    //if(dot(z,z) > 2*2) {
    if (tmp.x + tmp.y > 2 * 2) {
      break;
    }
  }
  
  float col = iter / (float)max_iter;
  float4 color = { 
    col * coord.x / (float)w,
    col * coord.y / (float)h, 
    col, 1.0f };
  write_imagef(im, coord, color);

  // Task 3: Navigate the Mandelbrot set! (WASD + zoom + rotation)

  ///////////////////////////////////////////////////////////////////////////
  // // Help for Mandelbrot set tasks:
  // 
  // float2 z = c; // c should be between about -1 and 1 for both coordinates
  // 
  // int iter = 0;
  // for (; iter < max_iter; ++iter)
  // {
  //     // TODO: z = z^2 + c
  //     // TODO: sufficient condition for divergence
  // }
  // 
  // float col = iter / (float)max_iter;
  // float4 color = { col * coord.x / (float)w, col * coord.y / (float)h, col, 1.0f };
  // write_imagef(im, coord, color);
  //
  ///////////////////////////////////////////////////////////////////////////
}