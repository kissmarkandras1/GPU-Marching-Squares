#include "MyApp.h"
#include "GLUtils.hpp"

#include <GL/GLU.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace cv;

bool CMyApp::InitGL()
{
    lookuptable.push_back(imread("lookup0.png", IMREAD_COLOR));
    lookuptable.push_back(imread("lookup01.png", IMREAD_COLOR));
    lookuptable.push_back(imread("lookup02.png", IMREAD_COLOR));
    lookuptable.push_back(imread("lookup03.png", IMREAD_COLOR));
    lookuptable.push_back(imread("lookup04.png", IMREAD_COLOR));
    lookuptable.push_back(imread("lookup05.png", IMREAD_COLOR));
    lookuptable.push_back(imread("lookup06.png", IMREAD_COLOR));
    lookuptable.push_back(imread("lookup07.png", IMREAD_COLOR));
    lookuptable.push_back(imread("lookup08.png", IMREAD_COLOR));
    lookuptable.push_back(imread("lookup0.png", IMREAD_COLOR));
    string line;

    ifstream inputFile("hdata2.txt");
    maxthreshold = FLT_MIN;
    minthreshold = FLT_MAX;

    while (getline(inputFile, line)) {
        vector<float> lineVector;
        stringstream s(line);
        float tmp;
        while (s >> tmp) {
            lineVector.push_back(tmp);
            if (tmp > maxthreshold)
                maxthreshold = tmp;
            if (tmp < minthreshold)
                minthreshold = tmp;
        }
        data.push_back(lineVector);
    }

    inputFile.close();

    maxthreshold += 50.0f;

    /*Mat elevMap = createElevationMap(data);

    for (float i = minthreshold; i < maxthreshold; i += 50.0f)
    {
        vector<vector<int>> toLookup = marchingSquares(data, i);
        Mat topMat = createLayer(toLookup);
        elevMap = mergeLayer(elevMap, topMat);
    }

    imwrite("output.png", elevMap);*/

  glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

  // Create texture
  texture = initTexture(texture_size, texture_size);
/*
  // fájlból betöltés
  texture1 = TextureFromFile("output.png");
  // mintavételezés beállításai
  glBindTexture(GL_TEXTURE_2D, texture1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // bilineáris szürés nagyításkor (ez az alapértelmezett)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // bilineáris szûrés a mipmap-ekböl kicsinyítéskor
  // mi legyen az eredmény, ha a textúrán kívülröl próbálunk mintát venni?
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // vízszintesen
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // függölegesen
  */
  //scale = 2.0;
  //center = glm::vec2(0, 0);
  max_iter = 50;

  return true;
}

bool CMyApp::InitCL()
{



  try
  {
    ///////////////////////////
    // Initialize OpenCL API //
    ///////////////////////////

    cl::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    // Try to get the sharing platform!
    bool create_context_success = false;
    for (auto platform : platforms) {
      // Next, create an OpenCL context on the platform.  Attempt to
      // create a GPU-based context.
      cl_context_properties contextProperties[] =
      {
  #ifdef _WIN32
        CL_CONTEXT_PLATFORM, (cl_context_properties)(platform)(),
        CL_GL_CONTEXT_KHR,   (cl_context_properties)wglGetCurrentContext(),
        CL_WGL_HDC_KHR,      (cl_context_properties)wglGetCurrentDC(),
  #elif defined( __GNUC__)
        CL_CONTEXT_PLATFORM, (cl_context_properties)(platform)(),
        CL_GL_CONTEXT_KHR,   (cl_context_properties)glXGetCurrentContext(),
        CL_GLX_DISPLAY_KHR,  (cl_context_properties)glXGetCurrentDisplay(),
  #elif defined(__APPLE__)
        //todo
  #endif
        0
      };

      // Create Context
      try {
        context = cl::Context(CL_DEVICE_TYPE_GPU, contextProperties);
        create_context_success = true;
        break;
      }
      catch (cl::Error error) {}
    }

    if (!create_context_success)
      throw cl::Error(CL_INVALID_CONTEXT, "Failed to create CL/GL shared context");

    // Create Command Queue
    cl::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
    command_queue = cl::CommandQueue(context, devices[0]);
    command_queuems = cl::CommandQueue(context, devices[0]);

    /////////////////////////////////
    // Load, then build the kernel //
    /////////////////////////////////

    // Read source file
    std::ifstream sourceFile("GLinterop.cl");
    std::string sourceCode(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));
    cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));

    // Make program of the source code in the context
    program = cl::Program(context, source);
    try {
      program.build(devices);
    }
    catch (cl::Error error) {
      std::cout << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]) << std::endl;
      throw error;
    }

    // Make kernel
    kernel_tex = cl::Kernel(program, "texture_kernel");

    // Create Mem Objs
    cl_tex_mem = cl::Image2DGL(context,
      CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, texture);


    // Read source file
    std::ifstream sourceFilems("marchingsquare.cl");
    std::string sourceCodems(std::istreambuf_iterator<char>(sourceFilems), (std::istreambuf_iterator<char>()));
    cl::Program::Sources sourcems(1, std::make_pair(sourceCodems.c_str(), sourceCodems.length() + 1));

    // Make program of the source code in the context
    programms = cl::Program(context, sourcems);
    try {
        programms.build(devices);
    }
    catch (cl::Error error) {
        std::cout << programms.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]) << std::endl;
        throw error;
    }

    // Make kernel
    kernelms = cl::Kernel(programms, "marchingsquare");

    // Create Mem Objs
    cl::Buffer buffer1(context, CL_MEM_READ_WRITE, data.size()*data[0].size() * sizeof(float));
    msinbuffer = buffer1;
    cl::Buffer buffer2(context, CL_MEM_READ_WRITE, (data.size()-1) * (data[0].size()-1) * sizeof(int));
    msoutbuffer = buffer2;

    // Query textures
    performTexQuery(); // Just to check..

    Mat elevMap = createElevationMap(data);

    for (float i = minthreshold; i < maxthreshold; i += 50.0f)
    {
        vector<vector<int>> toLookup = marchingSquares(data, i);
        Mat topMat = createLayer(toLookup);
        elevMap = mergeLayer(elevMap, topMat);
    }

    imwrite("output.png", elevMap);

    // fájlból betöltés
    texture1 = TextureFromFile("output.png");
    // mintavételezés beállításai
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // bilineáris szürés nagyításkor (ez az alapértelmezett)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // bilineáris szûrés a mipmap-ekböl kicsinyítéskor
    // mi legyen az eredmény, ha a textúrán kívülröl próbálunk mintát venni?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // vízszintesen
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // függölegesen
  }
  catch (cl::Error error)
  {
    std::cout << error.what() << "(" << oclErrorString(error.err()) << ")" << std::endl;
    return false;
  }
  return true;
}

void CMyApp::Clean()
{
  // after we have released the OpenCL references, we can delete the underlying OpenGL objects
  if (texture != 0)
  {
    glBindBuffer(GL_TEXTURE_RECTANGLE_ARB, texture);
    glDeleteBuffers(1, &texture);
  }
}

#pragma region Update (CL)

void CMyApp::computeTexture()
{
  // Set arguments to kernel
  kernel_tex.setArg(0, cl_tex_mem); // buffer
  kernel_tex.setArg(1, texture_size); // integer value
  kernel_tex.setArg(2, texture_size); // integer value
  kernel_tex.setArg(3, max_iter); // integer value

  // Run the kernel on specific ND range
  cl::NDRange global(texture_size, texture_size);
  command_queue.enqueueNDRangeKernel(kernel_tex, cl::NullRange, global, cl::NullRange);
}

void CMyApp::Update()
{
  static Uint32 last_time = SDL_GetTicks();
  delta_time = (SDL_GetTicks() - last_time) / 1000.0f;

  // CL
  try {
    cl::vector<cl::Memory> acquirable;
    acquirable.push_back(cl_tex_mem);

    // Acquire GL Objects
    command_queue.enqueueAcquireGLObjects(&acquirable);
    {
      // Perform computations
      computeTexture();

      // Wait for all computations to finish
      command_queue.finish();
    }
    // Release GL Objects
    command_queue.enqueueReleaseGLObjects(&acquirable);

  }
  catch (cl::Error error) {
    std::cout << error.what() << "(" << oclErrorString(error.err()) << ")" << std::endl;
    exit(1);
  }

  last_time = SDL_GetTicks();
}

#pragma endregion

#pragma region Render (GL)

void CMyApp::displayTexture(int w, int h)
{
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);

  glTexCoord2f(0, 0);
  glVertex2f(-1, 1);

  glTexCoord2f(0, 1);
  glVertex2f(-1, -1);

  glTexCoord2f(1, 1);
  glVertex2f(1, -1);

  glTexCoord2f(1, 0);
  glVertex2f(1, 1);

  glEnd();
  glDisable(GL_TEXTURE_2D);
}

void CMyApp::Render()
{
  // clear frame buffer (GL_COLOR_BUFFER_BIT) and the Z buffer (GL_DEPTH_BUFFER_BIT)
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  // GL
  displayTexture(texture_size, texture_size);
}

#pragma endregion

vector<vector<int>> CMyApp::marchingSquares(vector<vector<float>> input, float threshold) {
    vector<float> inputa;
    int* outputa = new int[(input.size() - 1) * (input[0].size() - 1)];

    for (int i = 0; i < input.size(); i++)
    {
        for (int j = 0; j < input[0].size(); j++)
        {
            inputa.push_back(input[i][j]);
        }
    }
    command_queuems.enqueueWriteBuffer(msinbuffer, CL_TRUE, 0, (input.size() * input[0].size() * sizeof(float)), inputa.data());
    command_queuems.finish();

    kernelms.setArg(0, msinbuffer);
    kernelms.setArg(1, msoutbuffer);
    kernelms.setArg(2, threshold);
    kernelms.setArg(3, input.size());

    cl::NDRange global(input.size(), input[0].size());
    command_queue.enqueueNDRangeKernel(kernelms, cl::NullRange, global, cl::NullRange);

    command_queue.enqueueReadBuffer(msoutbuffer, CL_TRUE, 0, (input[0].size() - 1)* (input[0].size() - 1)*sizeof(int), outputa);
    command_queuems.finish();

    vector<vector<int>> resultVector;

    int aind = 0;
    for (int i = 0; i < input.size()-1; i++)
    {
        vector<int> resultline;
        for (int j = 0; j < input[0].size()-1; j++)
        {
            resultline.push_back(outputa[aind]);
            aind++;
        }
        resultVector.push_back(resultline);
    }

    return resultVector;
}

Mat CMyApp::createElevationMap(vector<vector<float>> data) {
    Mat result;
    vector<Mat> lines;
    for (int i = 0; i < data.size(); i++)
    {
        lines.push_back(createElevationMapLine(data[i]));
    }

    Mat* linesp = &lines[0];

    vconcat(linesp, lines.size(), result);
    return result(Range(8,result.size().width-8), Range(8, result.size().height - 8));
}

Mat CMyApp::createElevationMapLine(vector<float> data) {
    Mat result = getElevationColor(data[0]);
    for (int i = 1; i < data.size(); i++)
    {
        Mat left = result;
        Mat right = getElevationColor(data[i]);
        hconcat(left, right, result);
    }
    return result;
}

Mat CMyApp::getElevationColor(float data) {
    Mat result = lookuptable[9];
    int level = (int)(255.0f*(1.0f - ((maxthreshold - data) / (maxthreshold - minthreshold))));
    result.setTo(level);

    return result;
}

Mat CMyApp::mergeLayer(Mat base, Mat top) {
    Mat topGray;
    cvtColor(top, topGray, COLOR_BGR2GRAY);
    top.copyTo(base, top > 0);
    return base;
}

Mat CMyApp::createLayer(vector<vector<int>> lookupvalues) {
    Mat result;
    vector<Mat> lines;
    for (int i = 0; i < lookupvalues.size(); i++)
    {
        lines.push_back(createLayerLine(lookupvalues[i]));
    }

    Mat* linesp = &lines[0];

    vconcat(linesp, lines.size(), result);
    return result;
}

Mat CMyApp::createLayerLine(vector<int> lookupline) {
    Mat result = doLookup(lookupline[0]);
    for (int i = 1; i < lookupline.size(); i++)
    {
        Mat left = result;
        Mat right = doLookup(lookupline[i]);
        hconcat(left, right, result);
    }
    return result;
}

Mat CMyApp::doLookup(int ind) {
    Mat result;

    switch (ind) {
    case 0:
        result = lookuptable[0];
        break;
    case 1:
        result = lookuptable[1];
        break;
    case 2:
        result = lookuptable[2];
        break;
    case 3:
        result = lookuptable[7];
        break;
    case 4:
        result = lookuptable[3];
        break;
    case 5:
        result = lookuptable[8];
        break;
    case 6:
        result = lookuptable[5];
        break;
    case 7:
        result = lookuptable[4];
        break;
    case 8:
        result = lookuptable[4];
        break;
    case 9:
        result = lookuptable[6];
        break;
    case 10:
        result = lookuptable[8];
        break;
    case 11:
        result = lookuptable[3];
        break;
    case 12:
        result = lookuptable[7];
        break;
    case 13:
        result = lookuptable[2];
        break;
    case 14:
        result = lookuptable[1];
        break;
    case 15:
        result = lookuptable[0];
        break;
    default:
        result = lookuptable[0];
        break;
    }

    return result;
}

#pragma region etc

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
  const float move_speed = 0.05;
  const float zoom_speed = 1.05;

  switch (key.keysym.sym)
  {
    // TODO

  case 'r':
    max_iter += 1;
    break;
  case 'f':
    max_iter -= 1;
    break;
  default:
    break;
  }
  if (max_iter < 1)
    max_iter = 1;
}

void CMyApp::KeyboardUp(SDL_KeyboardEvent& key)
{
}

void CMyApp::MouseMove(SDL_MouseMotionEvent& mouse)
{
}

void CMyApp::MouseDown(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseWheel(SDL_MouseWheelEvent& wheel)
{
}

// new windows width (_w) and height (_h)
void CMyApp::Resize(int _w, int _h)
{
  glViewport(0, 0, _w, _h);
  windowH = _h;
  windowW = _w;
}

CMyApp::CMyApp(void)
{
}

CMyApp::~CMyApp(void)
{
}

#pragma endregion
