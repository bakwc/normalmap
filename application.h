#include <QGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLFramebufferObject>
#include <QGLShaderProgram>

#include <memory>

struct TVertex {
    QVector3D Position;
    QVector3D Normal;
    QVector2D UV;
    QVector3D Tangent;
    QVector3D Bitangent;
};

class TApplication: public QGLWidget {
    Q_OBJECT
public:
    TApplication(QGLFormat format);
private:
    void initializeGL();
    void timerEvent(QTimerEvent*);
    void keyPressEvent(QKeyEvent* e);
    void paintEvent(QPaintEvent*);
    void RenderToFBO();
private:
    std::vector<TVertex> Obj;
    std::unique_ptr<QOpenGLBuffer> VertexBuff;
    std::unique_ptr<QOpenGLTexture> Texture;
    std::unique_ptr<QOpenGLTexture> NormalMap;
    int ObjectSize;
    std::unique_ptr<QOpenGLFramebufferObject> Fbo;
    std::unique_ptr<QOpenGLFramebufferObject> Fbo1;
    QGLShaderProgram Shader;
    float AngleX, AngleY, AngleZ;
    bool Paused;
};
