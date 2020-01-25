#ifndef GAPAINTER_H
#define GAPAINTER_H

#include "QRgb"
#include "QPointF"
#include "QColor"
#include "QPainter"
#include "QImage"

/*====================================================

  @FileName: GAPainter
  @Author  : Li Ang         psw.liang@foxmail.com
  @Time    : 2020. 01. 24
  Statement: This project is used for testing wheter GA could really work in
             high dimension spacing and fits the function or not.
             Wrting Just for kill the time, so the C ++ code may not written so well.
             Anyway, it works, and I have time to play Smash bros now!
             I put all of them in a Class file and may you use them successfully.

             Happy Lunar new year and  Pray for WU HAN

  \Class brief:
        Triangle: DNA withs the elements of triganle
        Scallop : stores DNA
        GAPainter: A painter than iterating and
                    gives back images to store
        ImageStore: A painter that revice image
                    and paint them in a single PNG
        GAPainterCollection:
                     it manages the thread of Painting

=====================================================*/

struct Triangle {
    QPointF point1;
    QPointF point2;
    QPointF point3;
    bool edge;
    // in fact you can delete the bool edge, it looks useless
    // I think the edge may be useful to paint a lines like hair of mouth
    // but I failed
    QColor color;
};


class Scallop {
public:
    Scallop(){

    }
    Scallop(int tri){
        Init(tri);
    }
    ~Scallop(){
        delete[] DNA;
        DNA = nullptr;
    }
    void Init(int tri){
        m_TRIANGLES_NUM = tri;
        DNA = new Triangle[m_TRIANGLES_NUM];
    }
    Scallop(const Scallop& t){
        this->update_flag = t.update_flag;
        this->fit_value = t.fit_value;
        if(DNA == nullptr)
            this->DNA = new Triangle[m_TRIANGLES_NUM];
        memcpy(this->DNA, t.DNA, sizeof(Triangle)*m_TRIANGLES_NUM);
    }
    Scallop& operator= (const Scallop& t){
      if(this == &t){
        return *this;
      }
      this->update_flag = t.update_flag;
      this->fit_value = t.fit_value;
      if(DNA == nullptr)
          this->DNA = new Triangle[m_TRIANGLES_NUM];
      memcpy(this->DNA, t.DNA, sizeof(Triangle)*m_TRIANGLES_NUM);
      return *this;
    }
    Scallop(Scallop&& t){
        this->fit_value = t.fit_value;
        this->update_flag = t.update_flag;
        this->DNA = t.DNA;
        t.DNA = nullptr;
    }
public:
    Triangle*   DNA                 = nullptr;
    double      fit_value           = 0.f;
    int         m_TRIANGLES_NUM     = 50;
    bool        update_flag         = false;// if it is need to repaint
};

class GAPainter: public QObject
{
    Q_OBJECT
public:
    GAPainter();
    ~GAPainter();
    void Init(int idx, QImage img);
    void runSingle();
    void stop();
private:

    void drawScallop(Scallop &scallop, QImage &dst_image);
    double calFit(Scallop &scallop);
    void randInit(Scallop* pop);
    int  rouletteSelect(double* accProb);
    void SortPop(Scallop* pop, int length);
    std::vector<int> SortOrder(double* data, int length);
private:
    void drawBestScallop(Scallop *pop, int generation);
    QImage getMaxScallop(Scallop *pop, int generation);
    void printPopulation(Scallop *pop);
private:
    void Crossover(Scallop *pop);
    void Mutation(Scallop *pop);
    void updateFit(Scallop *pop);
private:
    int         m_popNum             = 20;                  //Population size
    int         m_paintInterval      = 2000;                //the times of painting
    int         m_mutRATE            = 1;					//rate of the totally random mut
    int         m_generRate          = 5;                   //rate of changing slightly
    int         m_triangleNum        = 50;                  //the num of DNA in a Scallop
    Scallop*    m_pop                = nullptr;
    QImage      m_dstImage;                                 //target
    int         m_dstImageWidth;                            //image size

private:
    int     m_index         = 0;
    float   m_bestFitValue  = 0.f;
    int     m_count         = 0;
    bool    flag_runnning   = true;
signals:
    void sig_giveBackImage(int, QImage, float);
};

class QThread;
class ImageStore: public QObject{
    Q_OBJECT
public:
    ImageStore(int sz, int h, int w, int overlap = 0);
    ~ImageStore();
    void draw();
    void setImage(int index, QImage img, float fit);
private:
    int m_SZ      = 0;
    int m_H       = 0;
    int m_W       = 0;
    int m_overLap = 0;
    QImage m_resultImage;
    std::vector<QImage>* imgSet;
    std::vector<float>   fitSet;
    static int _drawCount;
    static int _saveCount;

};
class GAPainterCollection: public QObject{
    Q_OBJECT

public:
    GAPainterCollection(QString str, int divide);
    ~GAPainterCollection();
    void run();
private:
    int m_painterNum = 4;
    GAPainter*    m_painters           = nullptr;
    QThread*      m_threads            = nullptr;
    ImageStore*   m_imgStore           = nullptr;
};




#endif // GAPAINTER_H
