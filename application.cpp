#include <fstream>
#include <sstream>
#include <unordered_map>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "application.h"


constexpr int WINDOW_WIDTH = 600;
constexpr int WINDOW_HEIGHT = 600;


static std::vector<TVertex> LoadJsonModel(const QString& fileName) {
      QString val;
      QFile file;
      file.setFileName(fileName);
      file.open(QIODevice::ReadOnly | QIODevice::Text);
      val = file.readAll();
      file.close();

      QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());
      QJsonObject meshes = d.object().value("meshes").toArray().at(0).toObject();

      QJsonArray vertices = meshes.value("vertices").toArray();
      QJsonArray normals = meshes.value("normals").toArray();
      QJsonArray tangents = meshes.value("tangents").toArray();
      QJsonArray bitangents = meshes.value("bitangents").toArray();
      QJsonArray texturecoords = meshes.value("texturecoords").toArray().at(0).toArray();
      QJsonArray faces = meshes.value("faces").toArray();

      std::vector<TVertex> data;
      size_t objectSize = vertices.size() / 3;
      for (size_t i = 0; i < objectSize; ++i) {
          TVertex v;

          size_t idx = i * 3;
          size_t idx2 = i * 2;

          v.Position.setX(vertices.at(idx + 0).toDouble());
          v.Position.setY(vertices.at(idx + 1).toDouble());
          v.Position.setZ(vertices.at(idx + 2).toDouble());

          v.Normal.setX(normals.at(idx + 0).toDouble());
          v.Normal.setY(normals.at(idx + 1).toDouble());
          v.Normal.setZ(normals.at(idx + 2).toDouble());

          v.Tangent.setX(tangents.at(idx + 0).toDouble());
          v.Tangent.setY(tangents.at(idx + 1).toDouble());
          v.Tangent.setZ(tangents.at(idx + 2).toDouble());

          v.Bitangent.setX(bitangents.at(idx + 0).toDouble());
          v.Bitangent.setY(bitangents.at(idx + 1).toDouble());
          v.Bitangent.setZ(bitangents.at(idx + 2).toDouble());

          v.UV.setX(texturecoords.at(idx2 + 0).toDouble());
          v.UV.setY(texturecoords.at(idx2 + 1).toDouble());

          data.push_back(v);
      }

      std::vector<TVertex> dataFull;
      for (size_t i = 0; i < faces.size(); ++i) {
          QJsonArray face = faces.at(i).toArray();
          Q_ASSERT(face.size() == 3);
          for (size_t j = 0; j < face.size(); ++j) {
              int idx = face.at(j).toInt();
              dataFull.push_back(data[idx]);
          }
      }

      return dataFull;
}


TApplication::TApplication(QGLFormat format)
    : QGLWidget(format)
{
    this->setGeometry(this->x(), this->y(), WINDOW_WIDTH, WINDOW_HEIGHT);
    startTimer(50);
}

void TApplication::initializeGL() {

    // Init shaders
    if (!Shader.addShaderFromSourceFile(QGLShader::Vertex, "vert.glsl")) {
        qFatal("failed to load shader vert.glsl");
    }
    if (!Shader.addShaderFromSourceFile(QGLShader::Fragment, "frag.glsl")) {
        qFatal("failed to load shader frag.glsl");
    }
    if (!Shader.link()) {
        qFatal("failed to link shader planet");
    }
    if (!Shader.bind()) {
        qFatal("failed to bind shader");
    }

    // Init framebuffers
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(8);
    Fbo.reset(new QOpenGLFramebufferObject(WINDOW_WIDTH, WINDOW_HEIGHT, format));

    QOpenGLFramebufferObjectFormat format1;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    Fbo1.reset(new QOpenGLFramebufferObject(WINDOW_WIDTH, WINDOW_HEIGHT, format1));


    // Load model and textures

    Obj = LoadJsonModel("model.json");
    ObjectSize = Obj.size();

    VertexBuff.reset(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer));
    VertexBuff->create();
    VertexBuff->bind();
    VertexBuff->setUsagePattern(QOpenGLBuffer::StaticDraw);
    VertexBuff->allocate(Obj.data(), Obj.size() * sizeof(TVertex));
    VertexBuff->release();

    Texture.reset(new QOpenGLTexture(QImage("diffuse.jpg")));
    Texture->bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    Texture->release();

    NormalMap.reset(new QOpenGLTexture(QImage("normal.jpg")));
    NormalMap->bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    NormalMap->release();

    // Initial angles
    AngleX = 0;
    AngleY = 0;
    AngleZ = 0;

    Paused = false;
}

void TApplication::timerEvent(QTimerEvent*) {
    if (!Paused) {
        AngleX += 0.4;
        AngleY += 0.5;
        AngleZ += 0.6;
    }
    this->update();
}

void TApplication::keyPressEvent(QKeyEvent* e) {
    Paused = !Paused;
}

void TApplication::paintEvent(QPaintEvent*) {
    RenderToFBO();
    Q_ASSERT(Fbo && "FBO not initialized");
    QPainter p(this);
    QImage img = Fbo->toImage();
    p.drawImage(0, 0, img);
}

void TApplication::RenderToFBO() {
    Fbo->bind();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    Shader.bind();

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


    QMatrix4x4 view;
    view.lookAt(QVector3D(0.0, 0.0, 6.0), QVector3D(0.0, 0.0, 0.0), QVector3D(0.0, 1.0, 0.0));

    QMatrix4x4 projection;
    projection.perspective(45.0f, float(WINDOW_WIDTH) / float(WINDOW_HEIGHT), 0.1f, 10.0f);

    Shader.setUniformValue("projection", projection);
    Shader.setUniformValue("view", view);

    Shader.setUniformValue("lightIntensities", QVector3D(1.0, 1.0, 1.0));
    Shader.setUniformValue("lightPosition", QVector3D(-30.0, 30.0, 30.0));

    QMatrix4x4 model;
    model.translate(0, 0, 0.0);
    model.rotate(AngleX, 1.0, 0.0, 0.0);
    model.rotate(AngleY, 0.0, 1.0, 0.0);
    model.rotate(AngleZ, 0.0, 0.0, 1.0);
    model.scale(0.42);

    QMatrix3x3 normalMatrix = model.normalMatrix();

    Shader.setUniformValue("model", model);
    Shader.setUniformValue("normalMatrix", normalMatrix);


    VertexBuff->bind();

    GLint attribute;
    GLint offset = 0;

    attribute = Shader.attributeLocation("gl_Vertex");
    Shader.enableAttributeArray(attribute);
    Shader.setAttributeBuffer(attribute, GL_FLOAT, offset, 3, sizeof(TVertex));
    offset += sizeof(QVector3D);

    attribute = Shader.attributeLocation("gl_Normal");
    Shader.enableAttributeArray(attribute);
    Shader.setAttributeBuffer(attribute, GL_FLOAT, offset, 3, sizeof(TVertex));
    offset += sizeof(QVector3D);

    attribute = Shader.attributeLocation("vertTexCoord");
    Shader.enableAttributeArray(attribute);
    Shader.setAttributeBuffer(attribute, GL_FLOAT, offset, 2, sizeof(TVertex));
    offset += sizeof(QVector2D);

    attribute = Shader.attributeLocation("tangent");
    Shader.enableAttributeArray(attribute);
    Shader.setAttributeBuffer(attribute, GL_FLOAT, offset, 3, sizeof(TVertex));
    offset += sizeof(QVector3D);

    attribute = Shader.attributeLocation("bitangent");
    Shader.enableAttributeArray(attribute);
    Shader.setAttributeBuffer(attribute, GL_FLOAT, offset, 3, sizeof(TVertex));
    offset += sizeof(QVector3D);

    VertexBuff->release();

    Texture->bind(0, QOpenGLTexture::ResetTextureUnit);
    Shader.setUniformValue("texture", 0);
    NormalMap->bind(1, QOpenGLTexture::ResetTextureUnit);
    Shader.setUniformValue("normalMap", 1);

    glDrawArrays(GL_TRIANGLES, 0, 3 * ObjectSize);

    Shader.disableAttributeArray("gl_Vertex");
    Shader.disableAttributeArray("gl_Normal");
    Shader.disableAttributeArray("vertTexCoord");
    Shader.disableAttributeArray("tangent");
    Shader.disableAttributeArray("bitangent");

    Texture->release(0, QOpenGLTexture::ResetTextureUnit);
    NormalMap->release(1, QOpenGLTexture::ResetTextureUnit);


    Shader.disableAttributeArray("coord2d");
    Shader.disableAttributeArray("v_color");

    Shader.release();

/*
    /// DEBUG INFO

    QMatrix4x4 modelView = view * model;

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projection.data());
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(modelView.data());

//    glDisable(GL_DEPTH_TEST);

    glColor3f(1, 1, 1);
    glBegin(GL_LINES);
    for (size_t i = 0; i < Obj.size(); i += 3) {
        const TVertex& v1 = Obj[i];
        const TVertex& v2 = Obj[i + 1];
        const TVertex& v3 = Obj[i + 2];
        glVertex3f(v1.Position.x(), v1.Position.y(), v1.Position.z());
        glVertex3f(v2.Position.x(), v2.Position.y(), v2.Position.z());

        glVertex3f(v1.Position.x(), v1.Position.y(), v1.Position.z());
        glVertex3f(v3.Position.x(), v3.Position.y(), v3.Position.z());

        glVertex3f(v2.Position.x(), v2.Position.y(), v2.Position.z());
        glVertex3f(v3.Position.x(), v3.Position.y(), v3.Position.z());
    }
    glEnd();

    // normals
    glColor3f(0,0,1);
    glBegin(GL_LINES);
    for (size_t i = 0; i < Obj.size(); ++i) {
        const TVertex& v = Obj[i];
        QVector3D p = v.Position;
        p *= 1.02;
        glVertex3f(p.x(), p.y(), p.z());
        QVector3D n = v.Normal.normalized();
        p += n * 0.8;
        glVertex3f(p.x(), p.y(), p.z());
    }
    glEnd();

    // tangent
    glColor3f(1,0,0);
    glBegin(GL_LINES);
    for (size_t i = 0; i < Obj.size(); ++i) {
        const TVertex& v = Obj[i];
        QVector3D p = v.Position;
        p *= 1.02;
        glVertex3f(p.x(), p.y(), p.z());
        QVector3D n = v.Tangent.normalized();
        p += n * 0.8;
        glVertex3f(p.x(), p.y(), p.z());
    }
    glEnd();

    // bitangent
    glColor3f(0,1,0);
    glBegin(GL_LINES);
    for (size_t i = 0; i < Obj.size(); ++i) {
        const TVertex& v = Obj[i];
        QVector3D p = v.Position;
        p *= 1.02;
        glVertex3f(p.x(), p.y(), p.z());
        QVector3D n = v.Bitangent.normalized();
        p += n * 0.3;
        glVertex3f(p.x(), p.y(), p.z());
    }
    glEnd();
*/
    Fbo->release();
}
