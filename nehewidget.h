#ifndef NEHEWIDGET_H
#define NEHEWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>

typedef struct
{
    int r, g, b;
    GLfloat dist;       //距中心距离
    GLfloat angle;      //角度
}stars;

const GLuint num = 50;          //绘制星星总数

class NeHeWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit NeHeWidget(QWidget *parent = 0);
    ~NeHeWidget();
    
    void loadGLTextures();
    void timerEvent( QTimerEvent * );
    void buildFonts();

signals:
    
public slots:

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
    
private:
    bool fullscreen;

    GLfloat rTri;
    GLfloat rQuard;

    GLfloat xRot, yRot, zRot;
    GLfloat zoom;
    GLfloat xSpeed, ySpeed;
//    GLuint texture[3];
    GLuint filter;
    bool light;

    bool bLend;


    GLfloat tilt;
    GLfloat spin;
    GLuint loop;
    GLuint texture[1];
    bool twinkle;
    stars star[num];
};

#endif // NEHEWIDGET_H
