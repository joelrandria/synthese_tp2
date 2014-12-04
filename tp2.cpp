#include "App.h"

#include "Widgets/nvSDLContext.h"

#include "Mesh.h"
#include "MeshIO.h"

#include "Image.h"
#include "ImageArray.h"
#include "ImageIO.h"
#include "ImageManager.h"

#include "GL/GLQuery.h"
#include "GL/GLTexture.h"
#include "GL/GLBuffer.h"
#include "GL/GLVertexArray.h"
#include "GL/GLSampler.h"
#include "ProgramManager.h"

#include "MyFpsCamera.hpp"

class Tp2: public gk::App
{
  nv::SdlContext m_widgets;

  gk::GLProgram *m_program;
  gk::GLVertexArray *m_vao;

  gk::GLBuffer *m_vertex_buffer;
  gk::GLBuffer *m_index_buffer;
  int m_indices_size;

  gk::GLCounter *m_time;

  // Caméra
  MyFpsCamera _camera;
  bool _mouseMoveEnable;
  float _mouseMoveAngle;

  // Modèles
  gk::GLTexture* _bigguyTexture;
  gk::GLSampler* _bigguyTextureSampler;

public:

  Tp2()
    :gk::App(),

     // Caméra
     _camera(gk::Point(0, 0, 50),
	     gk::Vector(0, 0, -1),
	     gk::Vector(0, 1, 0),
	     gk::Perspective(60, 1, 0.01f, 1000.0f)),
     _mouseMoveEnable(false),
     _mouseMoveAngle(1),

     // Modèles & Textures
     _bigguyTexture(0),
     _bigguyTextureSampler(0)

  {
    gk::AppSettings settings;
    settings.setGLVersion(3,3);
    settings.setGLCoreProfile();
    settings.setGLDebugContext();

    // cree le contexte et une fenetre
    if(createWindow(512, 512, settings) < 0)
      closeWindow();

    m_widgets.init();
    m_widgets.reshape(windowWidth(), windowHeight());
  }
  ~Tp2()
  {
  }

  int init( )
  {
    loadShaders();
    loadMeshes();
    loadTextures();

    // mesure du temps de dessin
    m_time= gk::createTimer();

    // ok, tout c'est bien passe
    return 0;
  }

  void loadShaders()
  {
    gk::programPath("shaders");
    m_program= gk::createProgram("my_shader.glsl");
    if(m_program == gk::GLProgram::null())
      exit(-1);
  }
  void loadMeshes()
  {
    gk::Mesh* mesh = gk::MeshIO::readOBJ("bigguy.obj");
    if(mesh == NULL)
      exit(-1);

    m_vao = gk::createVertexArray();

    m_vertex_buffer = gk::createBuffer(GL_ARRAY_BUFFER, mesh->positions);
    glVertexAttribPointer(m_program->attribute("position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(m_program->attribute("position"));

    gk::GLBuffer* normalBuffer = gk::createBuffer(GL_ARRAY_BUFFER, mesh->normals);
    glVertexAttribPointer(m_program->attribute("normal"), 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(m_program->attribute("normal"));

    gk::GLBuffer* texCoords = gk::createBuffer(GL_ARRAY_BUFFER, mesh->texcoords);
    glVertexAttribPointer(m_program->attribute("texcoord"), 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(m_program->attribute("texcoord"));

    m_index_buffer= gk::createBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indices);
    m_indices_size= mesh->indices.size();

    delete mesh;

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }
  void loadTextures()
  {
    gk::Image* bigguyTexture = gk::readImage("bigguy_ambient_color.png");

    _bigguyTexture = gk::createTexture2D(gk::GLTexture::UNIT0, bigguyTexture, gk::TextureRGB16F);
    _bigguyTextureSampler = gk::createLinearSampler(GL_CLAMP_TO_BORDER);
  }

  int quit()
  {
    return 0;
  }

  // a redefinir pour utiliser les widgets.
  void processWindowResize( SDL_WindowEvent& event )
  {
    m_widgets.reshape(event.data1, event.data2);
  }
  // a redefinir pour utiliser les widgets.
  void processMouseButtonEvent( SDL_MouseButtonEvent& event )
  {
    m_widgets.processMouseButtonEvent(event);
  }
  // a redefinir pour utiliser les widgets.
  void processMouseMotionEvent( SDL_MouseMotionEvent& event )
  {
    m_widgets.processMouseMotionEvent(event);

    if (_mouseMoveEnable)
      {
	_camera.yaw(_mouseMoveAngle * -event.xrel);
	_camera.pitch(_mouseMoveAngle * event.yrel);
      }
  }
  // a redefinir pour utiliser les widgets.
  void processKeyboardEvent( SDL_KeyboardEvent& event )
  {
    m_widgets.processKeyboardEvent(event);
  }

  int draw( )
  {
    if(key(SDLK_ESCAPE))
      closeWindow();

    if(key('q'))
      {
	_camera.localTranslate(gk::Vector(-1, 0, 0));
      }
    if(key('d'))
      {
	_camera.localTranslate(gk::Vector(1, 0, 0));
      }
    if(key('z'))
      {
	_camera.localTranslate(gk::Vector(0, 0, -1));
      }
    if(key('s'))
      {
	_camera.localTranslate(gk::Vector(0, 0, 1));
      }

    if (key(SDLK_LCTRL))
      _mouseMoveEnable = true;
    else
      _mouseMoveEnable = false;

    if(key('r'))
      {
	key('r')= 0;
	// recharge et recompile les shaders
	gk::reloadPrograms();
      }
    if(key('c'))
      {
	key('c')= 0;
	// enregistre l'image opengl
	gk::writeFramebuffer("screenshot.png");
      }

    glViewport(0, 0, windowWidth(), windowHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // mesurer le temps d'execution
    m_time->start();

    // dessiner quelquechose
    glUseProgram(m_program->name);

    // parametrer le shader
    gk::Transform mvp = _camera.projectionTransform() * _camera.worldToViewTransform();

    m_program->uniform("mvpMatrix") = mvp.matrix();

    // selectionner un ensemble de buffers et d'attributs de sommets
    glBindVertexArray(m_vao->name);

    // texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(_bigguyTexture->target, _bigguyTexture->name);
    m_program->sampler("image") = 0;
    glBindSampler(0, _bigguyTextureSampler->name);

    // dessiner un maillage indexe
    glDrawElements(GL_TRIANGLES, m_indices_size, GL_UNSIGNED_INT, 0);

    // nettoyage
    glUseProgram(0);
    glBindVertexArray(0);

    // mesurer le temps d'execution
    m_time->stop();

    // afficher le temps d'execution
    {
      m_widgets.begin();
      m_widgets.beginGroup(nv::GroupFlags_GrowDownFromLeft);

      m_widgets.doLabel(nv::Rect(), m_time->summary("draw").c_str());

      m_widgets.endGroup();
      m_widgets.end();
    }

    // afficher le dessin
    present();
    // continuer
    return 1;
  }
};

int main()
{
  Tp2 app;
  app.run();

  return 0;
}
