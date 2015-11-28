#include <QGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLFramebufferObject>
#include <QGLShaderProgram>

#include <memory>

class TApplication: public QGLWidget {
    Q_OBJECT
public:
    TApplication(QGLFormat format);
private:
    void initializeGL();
    void timerEvent(QTimerEvent*);
    void paintEvent(QPaintEvent*);
    void RenderToFBO();
private:
    std::unique_ptr<QOpenGLBuffer> VertexBuff;
    std::unique_ptr<QOpenGLTexture> Texture;
    std::unique_ptr<QOpenGLTexture> NormalMap;
    int ObjectSize;
    std::unique_ptr<QOpenGLFramebufferObject> Fbo;
    std::unique_ptr<QOpenGLFramebufferObject> Fbo1;
    QGLShaderProgram Shader;
    float AngleX, AngleY, AngleZ;
};
