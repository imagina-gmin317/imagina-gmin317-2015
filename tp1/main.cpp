/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <iostream>
#include <stdlib.h>
#include <time.h>

#include "openglwindow.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>
#include <QKeyEvent>
#include <QtCore/qmath.h>

using namespace std;

#define NB_SQUARE_PER_LINE 239 //239 pour le test de la height map

//nb de carre par ligne +1 au carré = le nb de sommet totales
GLfloat points[(NB_SQUARE_PER_LINE+1)*(NB_SQUARE_PER_LINE+1)*3];

bool first_time;

//! [1]
class TriangleWindow : public OpenGLWindow
{
public:
    TriangleWindow();

    void initialize() Q_DECL_OVERRIDE;
    void render() Q_DECL_OVERRIDE;
    void generate_X_square_V1(int nb_square_per_ligne);
    void generate_X_square_With_Rand_Z();
    void generate_heighmap(char* pathToTheFile);

protected:
    void keyPressEvent( QKeyEvent * event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent * event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent * event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent * event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent* event)Q_DECL_OVERRIDE;

private:
    GLuint loadShader(GLenum type, const char *source);

    GLfloat m_X;
    GLfloat m_Y;
    GLfloat m_Z;

    GLfloat m_rotationX;
    GLfloat mouseRotationX;
    GLfloat m_rotationZ;
    GLfloat mouseRotationZ;

    GLfloat m_sens;

    int m_frame;
    int m_mode;

    bool mouseClick;
    GLfloat mousePosX;
    GLfloat mousePosY;

    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;

    QOpenGLShaderProgram *m_program;

};

TriangleWindow::TriangleWindow()
    : m_program(0)
    , m_frame(0),
      m_X(0),
      m_Y(1),
      m_Z(-50),
      m_rotationX(0),
      m_rotationZ(0),
      m_sens(1),
      m_mode(GL_LINES),
      mouseClick(false)
{
}
//! [1]

//! [2]
int main(int argc, char **argv)
{

    first_time = true;
    QGuiApplication app(argc, argv);

    QSurfaceFormat format;
    format.setSamples(16);

    TriangleWindow window;
    window.setFormat(format);
    window.resize(640, 480);
    window.show();

    window.setAnimating(true);

    return app.exec();
}
//! [2]


//! [3]
static const char *vertexShaderSource =
    "attribute highp vec4 posAttr;\n"
    "attribute lowp vec4 colAttr;\n"
    "varying lowp vec4 col;\n"
    "uniform highp mat4 matrix;\n"
    "void main() {\n"
    "   col = colAttr;\n"
    "   gl_Position = matrix * posAttr;\n"
    "}\n";

static const char *fragmentShaderSource =
    "varying lowp vec4 col;\n"
    "void main() {\n"
    "   gl_FragColor = col;\n"
    "}\n";
//! [3]

//! [4]
GLuint TriangleWindow::loadShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    return shader;
}

void TriangleWindow::initialize()
{
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();
    m_posAttr = m_program->attributeLocation("posAttr");
    m_colAttr = m_program->attributeLocation("colAttr");
    m_matrixUniform = m_program->uniformLocation("matrix");


    int MatSpec [4] = {1,1,1,1};
    int LightPos[4] = {0,0,30,1};

    glMaterialiv(GL_FRONT,GL_SPECULAR,MatSpec);
    //glMateriali(GL_FRONT,GL_SHININESS,100);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLightiv(GL_LIGHT0,GL_POSITION,LightPos);


    //glClearColor(0, 0, 0.5, 1);

}
//! [4]

//! [5]
void TriangleWindow::render()
{
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    QMatrix4x4 matrix;
    matrix.perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
    matrix.translate(m_X, m_Y, m_Z);
    //100.0f * m_frame / screen()->refreshRate()
    matrix.rotate(m_rotationX+mouseRotationX, 1, 0, 0);
    matrix.rotate(m_rotationZ+mouseRotationZ, 0, 0, 1);


    m_program->setUniformValue(m_matrixUniform, matrix);

    ///l'argument correspond au nombre de carre par ligne (ici c'est un carre de 15*15 carre donc 16*16 sommet)
    //generate_X_square_V1(15);

    ///Modifier la macro NB_SQUARE_PER_LINE ligne 56 pour changer le nombre de carre par ligne
    //generate_X_square_With_Rand_Z();

    ///pour cette fonction il faut mettre la macro NB_SQUARE_PER_LINE a 239 carre par ligne (donc 240 sommet par ligne)
    ///il faut aussi mettre le chemin absolue vers l'image de la height map

    generate_heighmap("C:/Users/Bastien/Desktop/Workspace_QT/imagina-gmin317-2015/tp1/heightmap-1.png");

    m_program->release();

    ++m_frame;
}
//! [5]

void TriangleWindow::keyPressEvent(QKeyEvent* event){

    switch(event->key()){
        case Qt::Key_T:
            m_Y -= 2.0f;
            cout << "y--" << endl;
            this->render();
            break;
        case Qt::Key_G:
            m_Y += 2.0f;
            cout << "y++" << endl;
            this->render();
            break;
        case Qt::Key_S:
            m_X += 2.0f;
            cout << "x++" << endl;
            this->render();
            break;
        case Qt::Key_F:
            m_X -= 2.0f;
            cout << "x--" << endl;
            this->render();
            break;
        case Qt::Key_E:
            m_Z += 2.0f;
            cout << "z++" << endl;
            this->render();
            break;
        case Qt::Key_D:
            m_Z -= 2.0f;
            cout << "Z--" << endl;
            this->render();
            break;
        case Qt::Key_Left:
            m_rotationZ = m_rotationZ - 1.5f;
            cout << "rotation sur Z" << endl;
            this->render();
            break;
        case Qt::Key_Right:
            m_rotationZ = m_rotationZ + 1.5f;
            cout << "rotation sur Z dans l'autre sens" << endl;
            this->render();
            break;
        case Qt::Key_Up:
            m_rotationX = m_rotationX - 1.5f;
            cout << "rotation sur X" << endl;
            this->render();
            break;
        case Qt::Key_Down:
            m_rotationX = m_rotationX + 1.5f;
            cout << "rotation sur X dans l'autre sens" << endl;
            this->render();
            break;
        case Qt::Key_Shift:
            if(m_mode == GL_TRIANGLES)
                m_mode = GL_LINES;
            else if(m_mode == GL_LINES)
                m_mode = GL_TRIANGLES;
            this->render();
            break;
    }
}

void TriangleWindow::mousePressEvent(QMouseEvent* event){
    mouseClick = true;
    QPoint point = event->pos();
    mousePosX = point.x();
    mousePosY = point.y();

}

void TriangleWindow::mouseReleaseEvent(QMouseEvent* event){
    mouseClick = false;
    m_rotationX += mouseRotationX;
    m_rotationZ += mouseRotationZ;
    mouseRotationX = 0;
    mouseRotationZ = 0;

}

void TriangleWindow::mouseMoveEvent(QMouseEvent* event){
    if(mouseClick){

        QPoint pts = event->pos();

        mouseRotationZ = pts.x() - mousePosX;
        mouseRotationX = pts.y() - mousePosY;
    }
}

void TriangleWindow::wheelEvent(QWheelEvent *event){

    if(event->delta() > 0)
        m_Z += 3.0f;
    else
        m_Z -= 3.0f;

}

void TriangleWindow::generate_X_square_V1(int nb_square_per_ligne){

    GLfloat x0 = -2.0f;
    GLfloat x = x0;
    GLfloat y = -2.0f;

    GLfloat sommet[nb_square_per_ligne*nb_square_per_ligne*2*3*2];//nb carre total * 2 = nb triangle encore * 3 nb de sommet *2 car 2 cooredonnées par sommet

    for(int i = 0; i < nb_square_per_ligne; ++i){

        for(int j = 0; j < nb_square_per_ligne; ++j){

            sommet[(i*nb_square_per_ligne*12)+j*12] = x;
            sommet[(i*nb_square_per_ligne*12)+j*12+1] = y;

            sommet[(i*nb_square_per_ligne*12)+j*12+2] = x;
            sommet[(i*nb_square_per_ligne*12)+j*12+3] = y+1.0f;

            sommet[(i*nb_square_per_ligne*12)+j*12+4] = x+1.0f;
            sommet[(i*nb_square_per_ligne*12)+j*12+5] = y;

            sommet[(i*nb_square_per_ligne*12)+j*12+6] = x;
            sommet[(i*nb_square_per_ligne*12)+j*12+7] = y+1.0f;

            sommet[(i*nb_square_per_ligne*12)+j*12+8] = x+1.0f;
            sommet[(i*nb_square_per_ligne*12)+j*12+9] = y;

            sommet[(i*nb_square_per_ligne*12)+j*12+10] = x+1.0f;
            sommet[(i*nb_square_per_ligne*12)+j*12+11] = y+1.0f;

            x += 1.0f;
        }

        y += 1.0f;
        x = x0;

    }

    GLfloat couleur[nb_square_per_ligne*nb_square_per_ligne*2*3*2*3];

    GLfloat R = 1.0f;
    GLfloat G = 0.0f;
    GLfloat B = 0.0f;

    for(int i = 0; i < nb_square_per_ligne*nb_square_per_ligne*2*3*2*3; i += 3){

        if(i % 36 == 0){
            R = 0.0f;
            G = 1.0f;
            B = 0.0f;
        }
        else if(i % 24 == 0){
            R = 1.0f;
            G = 0.0f;
            B = 0.0f;
        }
        else if (i % 12 == 0){
            R = 0.0f;
            G = 0.0f;
            B = 1.0f;
        }
        couleur[i] = R;
        couleur[i+1] = G;
        couleur[i+2] = B;
    }

    glVertexAttribPointer(m_posAttr, 2, GL_FLOAT, GL_FALSE, 0, sommet);
    glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, couleur);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLES, 0, nb_square_per_ligne*nb_square_per_ligne*2*3);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}


void TriangleWindow::generate_X_square_With_Rand_Z(){

    //nb de carre par ligne +1 au carré = le nb de sommet totales

    if(first_time){
        first_time = false;

        int heightMax = 3;
        int heightMin = 4;

        GLfloat x0 = -2.0f;
        GLfloat x = x0;
        GLfloat y = -2.0f;

        srand(time(NULL));

        //creation des sommets
        for(int i = 0; i < NB_SQUARE_PER_LINE+1; ++i){


            for(int j = 0; j < NB_SQUARE_PER_LINE+1; ++j){

                points[(i*(NB_SQUARE_PER_LINE+1)*3)+j*3] = x;
                points[(i*(NB_SQUARE_PER_LINE+1)*3)+j*3+1] = y;

                if(rand()%3 == 0)
                    points[(i*(NB_SQUARE_PER_LINE+1)*3)+j*3+2] = 3.0f*((float)(rand())/(float)(RAND_MAX));
                else
                       points[(i*(NB_SQUARE_PER_LINE+1)*3)+j*3+2] = 0.0f;
                x += 1.0f;
            }

            y += 1.0f;
            x = x0;

        }
    }

    GLfloat couleur1[NB_SQUARE_PER_LINE*3*3];//une couleurs par sommet
    GLfloat couleur2[NB_SQUARE_PER_LINE*3*3];
    GLfloat R = 0.0f;
    GLfloat G = 1.0f;
    GLfloat B = 0.0f;

    for(int i = 0; i < NB_SQUARE_PER_LINE*3*3; i += 3){

        if(i % 18 == 0){
            R = 0.0f;
            G = 1.0f;
            B = 0.0f;
        }
        else if(i % 12 == 0){
            R = 1.0f;
            G = 0.0f;
            B = 0.0f;
        }
        else if (i % 6 == 0){
            R = 0.0f;
            G = 0.0f;
            B = 1.0f;
        }

        couleur1[i] = R;
        couleur1[i+1] = G;
        couleur1[i+2] = B;

        couleur2[i] = G;
        couleur2[i+1] = B;
        couleur2[i+2] = R;
    }

    //utilisation des sommets pour faire des triangles
    //i correspond au deplacement sur les y
    for(int i = 0; i < NB_SQUARE_PER_LINE; ++i){

        GLfloat triangles1[NB_SQUARE_PER_LINE*3*3];
        GLfloat triangles2[NB_SQUARE_PER_LINE*3*3];

        //j correspond au deplacement sur les x
        for(int j = 0; j < NB_SQUARE_PER_LINE; ++j){

            //premier triangle
            triangles1[j*9] = points[(i*(NB_SQUARE_PER_LINE+1)*3)+(j+1)*3];
            triangles1[j*9+1] = points[(i*(NB_SQUARE_PER_LINE+1)*3)+(j+1)*3+1];
            triangles1[j*9+2] = points[(i*(NB_SQUARE_PER_LINE+1)*3)+(j+1)*3+2];

            triangles1[j*9+3] = points[((i+1)*(NB_SQUARE_PER_LINE+1)*3)+j*3];
            triangles1[j*9+4] = points[((i+1)*(NB_SQUARE_PER_LINE+1)*3)+j*3+1];
            triangles1[j*9+5] = points[((i+1)*(NB_SQUARE_PER_LINE+1)*3)+j*3+2];

            triangles1[j*9+6] = points[(i*(NB_SQUARE_PER_LINE+1)*3)+j*3];
            triangles1[j*9+7] = points[(i*(NB_SQUARE_PER_LINE+1)*3)+j*3+1];
            triangles1[j*9+8] = points[(i*(NB_SQUARE_PER_LINE+1)*3)+j*3+2];


            //deuxieme triangle
            triangles2[j*9] = points[((i+1)*(NB_SQUARE_PER_LINE+1)*3)+(j+1)*3];
            triangles2[j*9+1] = points[((i+1)*(NB_SQUARE_PER_LINE+1)*3)+(j+1)*3+1];
            triangles2[j*9+2] = points[((i+1)*(NB_SQUARE_PER_LINE+1)*3)+(j+1)*3+2];

            triangles2[j*9+3] = points[((i+1)*(NB_SQUARE_PER_LINE+1)*3)+j*3];
            triangles2[j*9+4] = points[((i+1)*(NB_SQUARE_PER_LINE+1)*3)+j*3+1];
            triangles2[j*9+5] = points[((i+1)*(NB_SQUARE_PER_LINE+1)*3)+j*3+2];

            triangles2[j*9+6] = points[(i*(NB_SQUARE_PER_LINE+1)*3)+(j+1)*3];
            triangles2[j*9+7] = points[(i*(NB_SQUARE_PER_LINE+1)*3)+(j+1)*3+1];
            triangles2[j*9+8] = points[(i*(NB_SQUARE_PER_LINE+1)*3)+(j+1)*3+2];

        }
        //afficahge de triangle1
        glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, 0, triangles1);
        glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, couleur1);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glDrawArrays(m_mode, 0, NB_SQUARE_PER_LINE*3);

        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);

        //afficahge de triangle2
        glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, 0, triangles2);
        glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, couleur2);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glDrawArrays(m_mode, 0, NB_SQUARE_PER_LINE*3);

        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
    }
}


void TriangleWindow::generate_heighmap(char* pathToTheFile){

    //nb de carre par ligne +1 au carré = le nb de sommet totales

    if(first_time){
        first_time = false;

        QImage image = QImage(pathToTheFile);

        GLfloat x0 = -100.0f;
        GLfloat x = x0;
        GLfloat y = -100.0f;

        //creation des sommets
        for(int i = 0; i < NB_SQUARE_PER_LINE+1; ++i){


            for(int j = 0; j < NB_SQUARE_PER_LINE+1; ++j){

                points[(i*(NB_SQUARE_PER_LINE+1)*3)+j*3] = x;
                points[(i*(NB_SQUARE_PER_LINE+1)*3)+j*3+1] = y;

                points[(i*(NB_SQUARE_PER_LINE+1)*3)+j*3+2] = qGray(image.pixel(i,j))/10;

                x += 1.0f;
            }

            y += 1.0f;
            x = x0;

        }
    }

    GLfloat couleur1[NB_SQUARE_PER_LINE*3*3];//une couleurs par sommet
    GLfloat couleur2[NB_SQUARE_PER_LINE*3*3];
    GLfloat R = 0.0f;
    GLfloat G = 1.0f;
    GLfloat B = 0.0f;

    //utilisation des sommets pour faire des triangles
    //i correspond au deplacement sur les y
    for(int i = 0; i < NB_SQUARE_PER_LINE; ++i){

        GLfloat triangles1[NB_SQUARE_PER_LINE*3*3];
        GLfloat triangles2[NB_SQUARE_PER_LINE*3*3];

        //j correspond au deplacement sur les x
        for(int j = 0; j < NB_SQUARE_PER_LINE; ++j){

            //premier triangle
            triangles1[j*9] = points[(i*(NB_SQUARE_PER_LINE+1)*3)+(j+1)*3];
            triangles1[j*9+1] = points[(i*(NB_SQUARE_PER_LINE+1)*3)+(j+1)*3+1];
            triangles1[j*9+2] = points[(i*(NB_SQUARE_PER_LINE+1)*3)+(j+1)*3+2];

            triangles1[j*9+3] = points[((i+1)*(NB_SQUARE_PER_LINE+1)*3)+j*3];
            triangles1[j*9+4] = points[((i+1)*(NB_SQUARE_PER_LINE+1)*3)+j*3+1];
            triangles1[j*9+5] = points[((i+1)*(NB_SQUARE_PER_LINE+1)*3)+j*3+2];

            triangles1[j*9+6] = points[(i*(NB_SQUARE_PER_LINE+1)*3)+j*3];
            triangles1[j*9+7] = points[(i*(NB_SQUARE_PER_LINE+1)*3)+j*3+1];
            triangles1[j*9+8] = points[(i*(NB_SQUARE_PER_LINE+1)*3)+j*3+2];


            //deuxieme triangle
            triangles2[j*9] = points[((i+1)*(NB_SQUARE_PER_LINE+1)*3)+(j+1)*3];
            triangles2[j*9+1] = points[((i+1)*(NB_SQUARE_PER_LINE+1)*3)+(j+1)*3+1];
            triangles2[j*9+2] = points[((i+1)*(NB_SQUARE_PER_LINE+1)*3)+(j+1)*3+2];

            triangles2[j*9+3] = points[((i+1)*(NB_SQUARE_PER_LINE+1)*3)+j*3];
            triangles2[j*9+4] = points[((i+1)*(NB_SQUARE_PER_LINE+1)*3)+j*3+1];
            triangles2[j*9+5] = points[((i+1)*(NB_SQUARE_PER_LINE+1)*3)+j*3+2];

            triangles2[j*9+6] = points[(i*(NB_SQUARE_PER_LINE+1)*3)+(j+1)*3];
            triangles2[j*9+7] = points[(i*(NB_SQUARE_PER_LINE+1)*3)+(j+1)*3+1];
            triangles2[j*9+8] = points[(i*(NB_SQUARE_PER_LINE+1)*3)+(j+1)*3+2];

        }

        for(int k = 0; k < NB_SQUARE_PER_LINE*3; ++k){

            if(triangles1[k*3+2] > 18.0f){//blanc
               R = 1.0f;
               G = 1.0f;
               B = 1.0f;
            }
            else if (triangles1[k*3+2] > 7.0f){//marron
                R = 88.0f/255.0f;
                G = 41.0f/255.0f;
                B = 0.0f;
            }
            else if (triangles1[k*3+2] > 3.0f){//vert
                R = 20.0f/255.0f;
                G = 148.0f/255.0f;
                B = 20.0f/255.0f;
            }
            else if (triangles1[k*3+2] > 2.5f){//jaune
                R = 255.0f/255.0f;
                G = 222.0f/255.0f;
                B = 117.0f/255.0f;
            }
            else if (triangles1[k*3+2] > 1.0f){//bleu clair
                R = 44.0f/255.0f;
                G = 117.0f/255.0f;
                B = 1.0f;
            }
            else{//bleu foncé
                R = 15.0f/255.0f;
                G = 5.0f/255.0f;
                B = 107.0f/255.0f;
            }

            couleur1[k*3] = R;
            couleur1[k*3+1] = G;
            couleur1[k*3+2] = B;

            if(triangles2[k*3+2] > 18.0f){
               R = 1.0f;
               G = 1.0f;
               B = 1.0f;
            }
            else if (triangles2[k*3+2] > 7.0f){
                R = 88.0f/255.0f;
                G = 41.0f/255.0f;
                B = 0.0f;
            }
            else if (triangles2[k*3+2] > 3.0f){
                R = 20.0f/255.0f;
                G = 148.0f/255.0f;
                B = 20.0f/255.0f;
            }
            else if (triangles2[k*3+2] > 2.0f){
                R = 255.0f/255.0f;
                G = 222.0f/255.0f;
                B = 117.0f/255.0f;
            }
            else if (triangles2[k*3+2] > 1.0f){//bleu clair
                R = 44.0f/255.0f;
                G = 117.0f/255.0f;
                B = 1.0f;
            }
            else{//bleu foncé
                R = 15.0f/255.0f;
                G = 5.0f/255.0f;
                B = 107.0f/255.0f;
            }

            couleur2[k*3] = R;
            couleur2[k*3+1] = G;
            couleur2[k*3+2] = B;
        }

        //afficahge de triangle1
        glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, 0, triangles1);
        glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, couleur1);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glDrawArrays(m_mode, 0, NB_SQUARE_PER_LINE*3);

        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);

        //afficahge de triangle2
        glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, 0, triangles2);
        glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, couleur2);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glDrawArrays(m_mode, 0, NB_SQUARE_PER_LINE*3);

        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
    }

}
