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

#include "openglwindow.h"

#include <QtCore/QCoreApplication>

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLPaintDevice>
#include <QtGui/QPainter>
#include <QKeyEvent>

//! [1]
OpenGLWindow::OpenGLWindow(QWindow *parent)
    : QWindow(parent)
    , m_update_pending(false)
    , m_animating(false)
    , m_context(0)
    , m_device(0)
{
    setSurfaceType(QWindow::OpenGLSurface);
}
//! [1]

OpenGLWindow::~OpenGLWindow()
{
    delete m_device;
}
//! [2]
void OpenGLWindow::render(QPainter *painter)
{
    Q_UNUSED(painter);
}

void OpenGLWindow::initialize()
{
}

void OpenGLWindow::render()
{
    if (!m_device)
        m_device = new QOpenGLPaintDevice;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_device->setSize(size());

    QPainter painter(m_device);
    render(&painter);
}
//! [2]

//! [3]
void OpenGLWindow::renderLater()
{
    if (!m_update_pending) {
        m_update_pending = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

bool OpenGLWindow::event(QEvent *event)
{
    switch (event->type())
    {
      case QEvent::UpdateRequest:
        m_update_pending = false;
        renderNow();
        return true;
      case QEvent::Wheel:
      {
        QWheelEvent* ev = (QWheelEvent*)event;
        if(ev->delta()>0)
            m_z += 20.0f;
        else if (ev->delta() < 0)
            m_z -= 20.0f;
        renderNow();
        return true;
      }
      case QKeyEvent::KeyPress:
      {
        switch(((QKeyEvent*)event)->key())
        {
          case Qt::Key_S:
            m_x -= 5.0f;
            break;
          case Qt::Key_F:
            m_x += 5.0f;
            break;
          case Qt::Key_E:
            m_y += 5.0f;
            break;
          case Qt::Key_D:
            m_y -= 5.0f;
            break;
          case Qt::Key_Minus:
            m_z -= 5.0f;
            break;
          case Qt::Key_Plus:
            m_z += 5.0f;
            break;
          case Qt::Key_Left:
            m_rotate_x = m_rotate_x - 10.0f;
            break;
          case Qt::Key_Right:
            m_rotate_x = m_rotate_x + 10.0f;
            break;
          case Qt::Key_Down:
            m_rotate_y = m_rotate_y - 10.0f;
            break;
          case Qt::Key_Up:
            m_rotate_y = m_rotate_y + 10.0f;
            break;
          case Qt::Key_1:
            m_affichage = GL_TRIANGLES;
            break;
          case Qt::Key_2:
            m_affichage = GL_LINES;
            break;
          case Qt::Key_3:
            m_affichage = GL_LINE_LOOP;
            break;
          default:
            return QWindow::event(event);
        }
        renderNow();
        return true;
      }
      default:
        return QWindow::event(event);
    }
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent *mouseEvent)
{
    if(mouseEvent->buttons() & Qt::LeftButton)
    {
        if(mouseEvent->x() > m_old_r_x)
            m_rotate_x = m_rotate_x + 3.0f;
        else if(mouseEvent->x() < m_old_r_x)
            m_rotate_x = m_rotate_x - 3.0f;
    }

    if(mouseEvent->buttons() & Qt::RightButton)
    {
        if(mouseEvent->x() > m_old_r_x)
            m_rotate_y = m_rotate_y + 3.0f;
        else if(mouseEvent->x() < m_old_r_x)
            m_rotate_y = m_rotate_y - 3.0f;
    }

    m_old_r_x = mouseEvent->x();
    m_old_r_y = mouseEvent->x();
}

void OpenGLWindow::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}
//! [3]

//! [4]
void OpenGLWindow::renderNow()
{
    if (!isExposed())
        return;

    bool needsInitialize = false;

    if (!m_context) {
        m_context = new QOpenGLContext(this);
        m_context->setFormat(requestedFormat());
        m_context->create();

        needsInitialize = true;
    }

    m_context->makeCurrent(this);

    if (needsInitialize) {
        initializeOpenGLFunctions();
        initialize();
    }

    render();

    m_context->swapBuffers(this);

    if (m_animating)
        renderLater();
}
//! [4]

//! [5]
void OpenGLWindow::setAnimating(bool animating)
{
    m_animating = animating;

    if (animating)
        renderLater();
}
//! [5]

