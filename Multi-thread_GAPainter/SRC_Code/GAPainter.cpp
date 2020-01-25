#include "GAPainter.h"
#include <iostream>
#include <algorithm>
#include <string>
#include "QThread"
#include "QDebug"
void GAPainter::drawScallop(Scallop &scallop, QImage &dst_image)
{
    dst_image.fill(0);
    QPainter painter(&dst_image);
    for(int index = 0; index < m_triangleNum; index++){
        if(!scallop.DNA[index].edge)
            painter.setPen(Qt::NoPen);
        QPointF contour[3] = { scallop.DNA[index].point1,
                                scallop.DNA[index].point2,
                                scallop.DNA[index].point3 };

        //paint triangles
        painter.setBrush(scallop.DNA[index].color);
        painter.drawPolygon(contour, 3);
    }

}


double GAPainter::calFit(Scallop &scallop)
{
    QImage dst_image(m_dstImageWidth, m_dstImageWidth, QImage::Format_RGBA8888 );
    double fit_result = 0.0;
    drawScallop(scallop, dst_image);

    int h = dst_image.height();
    int w = dst_image.width();

    int fullLength = h*w;
    int bLength = fullLength*4;

    uchar* pideal = m_dstImage.bits();
    uchar* pdst = dst_image.bits();

    for(int i = 0; i < bLength; i += 4){
        fit_result += sqrt(
                            (pideal[i] - pdst[i])    *(pideal[i] - pdst[i])    +
                            (pideal[i+1] - pdst[i+1])*(pideal[i+1] - pdst[i+1])+
                            (pideal[i+2] - pdst[i+2])*(pideal[i+2] - pdst[i+2])
                );
    }
    return bLength / fit_result;
}

void GAPainter::randInit(Scallop *pop)
{
    for (int i = 0; i<m_popNum; i++){
        for (int j = 0; j < m_triangleNum; j++){
            pop[i].DNA[j].point1 = QPointF(rand() % m_dstImageWidth, rand() % m_dstImageWidth);
            pop[i].DNA[j].point2 = QPointF(rand() % m_dstImageWidth, rand() % m_dstImageWidth);
            pop[i].DNA[j].point3 = QPointF(rand() % m_dstImageWidth, rand() % m_dstImageWidth);
            pop[i].DNA[j].color  = QColor(rand() % 255, rand() % 255, rand() % 255, rand() % 255);
            pop[i].DNA[i].edge = false;
        }
        pop[i].fit_value = calFit(pop[i]);
        pop[i].update_flag = false;
    }
}

int GAPainter::rouletteSelect(double *accProb)
{
    double m = rand()/double(RAND_MAX);
    float Probability_Total = 0;

    for(int i = 0; i < m_popNum; i++){
        Probability_Total += accProb[i];
        if(Probability_Total >= m){
             return  i;
         }
    }
    return 0;
}

void GAPainter::SortPop(Scallop *pop, int length)
{
    for(int i = 1; i < length; i++){
        auto temp = pop[i];
        int j = i-1;
        while(j >=0 && temp.fit_value > pop[j].fit_value)
        {
            pop[j+1] = pop[j];
            j--;
        }
        pop[j+1] = temp;
    }
}

std::vector<int> GAPainter::SortOrder(double *data, int length)
{
    std::vector<int> order(length);
    for(int i = 0; i < length; i++){
        order[i] = i;
    }
    for(int i = 1; i < length; i++){
        auto temp = data[i];
        auto tempindex = order[i];
        int j = i-1;
        while(j >=0 && temp > data[j])
        {
            data[j+1] = data[j];
            order[j+1] = order[j];
            j--;
        }
        data[j+1] = temp;
        order[j+1] = tempindex;
    }
    return order;
}


void GAPainter::Crossover(Scallop *pop)
{
    double fitSum = 0.f;
    double fitMin = 1000000.f;
    for(int i = 0; i < m_popNum; i++){
        fitSum += pop[i].fit_value;
        fitMin = (pop[i].fit_value < fitMin) ? pop[i].fit_value :fitMin;
    }
    fitSum -= fitMin * m_popNum;
    double* prob = new double [m_popNum];
    for(int i = 0; i < m_popNum; i++){
        prob[i] = (pop[i].fit_value - fitMin) / fitSum;
    }

    auto order = SortOrder(prob, m_popNum);
   std::vector<int> backOrder{1};
   backOrder.assign(order.begin() + m_popNum/2, order.end());

   for(const auto& childPos: backOrder){
       int fatherPos = rouletteSelect(prob);
       int motherPos = rouletteSelect(prob);

       int genepos = rand()%m_triangleNum;
       for(int j = 0; j < genepos; j++){
           pop[childPos].DNA[j] = pop[fatherPos].DNA[j];
       }
       for(int j = genepos; j < m_triangleNum; j++){
           pop[childPos].DNA[j] = pop[motherPos].DNA[j];
       }
       pop[childPos].update_flag = true;
   }
    delete[] prob;
}

void GAPainter::Mutation(Scallop *pop)
{
    int mutaionNum = 5;
    int generNum = 10;
    int gene_index;

    std::pair<int, double> maxScallop = std::pair<int, double>(0, pop[0].fit_value);
    for (int i = 0; i < m_popNum; i++)
        maxScallop = (maxScallop.second > pop[i].fit_value) ? maxScallop : std::pair<int, double>(i, pop[i].fit_value);

    for (int i = 0; i < m_popNum; i++){
//        if(i == maxScallop.first)//留最好的不变异，保证不会因为变异而变差
//            continue;
        if (rand() % 100 <= m_mutRATE){
            for(int n = 0; n < mutaionNum; n++){

                gene_index = rand() % m_triangleNum;

                pop[i].DNA[gene_index].color = QColor(rand() % 255, rand() % 255, rand() % 255, rand() % 255);
                pop[i].DNA[gene_index].point1 = QPointF(rand() % m_dstImageWidth, rand() % m_dstImageWidth);
                pop[i].DNA[gene_index].point2 = QPointF(rand() % m_dstImageWidth, rand() % m_dstImageWidth);
                pop[i].DNA[gene_index].point3 = QPointF(rand() % m_dstImageWidth, rand() % m_dstImageWidth);
                pop[i].DNA[gene_index].edge = rand()%100 < 20 ? true : false;//only here could get edge
            }

           //更新标志置为true
            pop[i].update_flag = true;
            continue;
        }
        if(rand() % 100 <= m_generRate){
            for(int n = 0; n < generNum; n++){
                int colorRange = 10;
                int colorRangeHalf = colorRange/2;
                int posRange = 20;
                int halfRange = posRange/2;
                gene_index = rand() % m_triangleNum;
                auto& color = pop[i].DNA[gene_index].color;
                uchar r = color.red()  + rand() % colorRange - colorRangeHalf;
                uchar g = color.green()  + rand() % colorRange - colorRangeHalf;
                uchar b = color.blue()  + rand() % colorRange - colorRangeHalf;
                uchar a = color.alpha()  + rand() % colorRange - colorRangeHalf;
                color.setRgb(r, g, b, 255);
                pop[i].DNA[gene_index].point1 += QPointF(rand() % posRange - halfRange, rand() % posRange -halfRange);
                if(pop[i].DNA[gene_index].point1.x() >= m_dstImageWidth)
                    pop[i].DNA[gene_index].point1.setX(m_dstImageWidth-1);
                if(pop[i].DNA[gene_index].point1.y() >= m_dstImageWidth)
                    pop[i].DNA[gene_index].point1.setY(m_dstImageWidth-1);
                if(pop[i].DNA[gene_index].point1.x() < 0)
                    pop[i].DNA[gene_index].point1.setX(0);
                if(pop[i].DNA[gene_index].point1.y() < 0)
                    pop[i].DNA[gene_index].point1.setY(0);

                pop[i].DNA[gene_index].point2 +=  QPointF(rand() % posRange - halfRange, rand() % posRange -halfRange);
                if(pop[i].DNA[gene_index].point2.x() >= m_dstImageWidth)
                    pop[i].DNA[gene_index].point2.setX(m_dstImageWidth-1);
                if(pop[i].DNA[gene_index].point2.y() >= m_dstImageWidth)
                    pop[i].DNA[gene_index].point2.setY(m_dstImageWidth-1);
                if(pop[i].DNA[gene_index].point2.x() < 0)
                    pop[i].DNA[gene_index].point2.setX(0);
                if(pop[i].DNA[gene_index].point2.y() < 0)
                    pop[i].DNA[gene_index].point2.setY(0);

                pop[i].DNA[gene_index].point3 +=  QPointF(rand() % posRange - halfRange, rand() % posRange -halfRange);
                if(pop[i].DNA[gene_index].point3.x() >= m_dstImageWidth)
                    pop[i].DNA[gene_index].point3.setX(m_dstImageWidth-1);
                if(pop[i].DNA[gene_index].point3.y() >= m_dstImageWidth)
                    pop[i].DNA[gene_index].point3.setY(m_dstImageWidth-1);
                if(pop[i].DNA[gene_index].point3.x() < 0)
                    pop[i].DNA[gene_index].point3.setX(0);
                if(pop[i].DNA[gene_index].point3.y() < 0)
                    pop[i].DNA[gene_index].point3.setY(0);

                pop[i].DNA[gene_index].edge = rand()%100 < 20 ? true : false;//only here could get edge
            }
            pop[i].update_flag = true;
        }
    }
}

void GAPainter::drawBestScallop(Scallop *pop, int generation)
{
    //寻找当代种群适应度最大扇贝
    std::pair<int, double> max_scallop = std::pair<int, double>(0, pop[0].fit_value);
    for (int i = 0; i < m_popNum; i++)
        max_scallop = (max_scallop.second >= pop[i].fit_value) ? max_scallop : std::pair<int, double>(i, pop[i].fit_value);

    //绘制该扇贝
    QImage dst_image(m_dstImageWidth, m_dstImageWidth, QImage::Format_RGBA8888);
    dst_image.fill(0);
    drawScallop(pop[max_scallop.first], dst_image);

    std::string fitness_str = std::to_string(pop[max_scallop.first].fit_value), generation_str = std::to_string(generation);
    std::string str = "C:\\Users\\Liang\\Desktop\\GA\\the" + generation_str + "Yome"+ fitness_str +".png";
    dst_image.save( QString::fromStdString(str));
}

QImage GAPainter::getMaxScallop(Scallop *pop, int generation)
{
    std::pair<int, double> max_scallop = std::pair<int, double>(0, pop[0].fit_value);
    for (int i = 0; i < m_popNum; i++)
        max_scallop = (max_scallop.second >= pop[i].fit_value) ? max_scallop : std::pair<int, double>(i, pop[i].fit_value);
    m_bestFitValue = pop[max_scallop.first].fit_value;
    QImage dst_image(m_dstImageWidth, m_dstImageWidth, QImage::Format_RGBA8888);
    dst_image.fill(0);
    drawScallop(pop[max_scallop.first], dst_image);
    return dst_image;
}

GAPainter::GAPainter()
{

}

GAPainter::~GAPainter()
{
    if(m_pop != nullptr){
        delete[] m_pop;
        m_pop = nullptr;
    }
}

void GAPainter::Init(int idx, QImage img)
{
    m_index = idx;
    m_pop = new Scallop[m_popNum];
    for(int  i = 0; i < m_popNum; i++){
        m_pop[i].Init(m_triangleNum );
    }
    m_dstImage = img;
    m_dstImageWidth = m_dstImage.height();
    //qDebug()<<GAPainter::Init;
    randInit(m_pop);
}

void GAPainter::runSingle()
{
    while(flag_runnning){

        for (int i = 0; i < m_popNum; i++)
            m_pop[i].update_flag = false;

            Crossover(m_pop);
            Mutation(m_pop);
            updateFit(m_pop);

            if (m_count==1||m_count%1000 == 0)
            qDebug()<<m_index<<"it comes to the :"<< m_count;
            //printPopulation(cur_pop);

            //emit the image to the painting part
            if (m_count==1||m_count % m_paintInterval == 0){
                emit  sig_giveBackImage(m_index, getMaxScallop(m_pop, m_count), m_bestFitValue);
            }
            m_count++;
    }
}

void GAPainter::stop()
{
    flag_runnning = false;
}

void GAPainter::printPopulation(Scallop *pop)
{
    for (int i = 0; i < m_popNum; i++)
    {
        if(pop[i].update_flag)
            printf("%d: fit_value=%f updated\n", i, pop[i].fit_value);
        else
            printf("%d: fit_value=%f\n", i, pop[i].fit_value);
    }
}

void GAPainter::updateFit(Scallop *pop)
{
    for (int i = 0; i < m_popNum; i++)
    {
        if(pop[i].update_flag)
            pop[i].fit_value = calFit(pop[i]);
    }
}

GAPainterCollection::GAPainterCollection(QString str, int divide)
{
    m_painterNum = divide * divide;
    m_painters = new GAPainter[m_painterNum];
    m_threads = new QThread[m_painterNum];
    int H = 144;
    int W = 144;
    int overlap = 10;
    int halfOverLap = overlap/2;
    m_imgStore = new ImageStore(m_painterNum, H, W, overlap);
    QImage img = QImage(str).scaled(H, W).convertToFormat(QImage::Format_RGBA8888);
    int h = H/divide + overlap;
    int w = W/divide + overlap;
    for(int n = 0; n < m_painterNum; n++){
        int i = n / divide;
        int j = n % divide;
        int x = 0;
        int y = 0;
        if( i == 0)
            x = 0;
        else if(i == divide-1)
            x = i*w - overlap - overlap;
        else
            x = i*w - overlap - halfOverLap;

        if( j == 0)
            y = 0;
        else if(j == divide-1)
            y = j*h - overlap - overlap;
        else
            y = j*h - overlap - halfOverLap;

        qDebug()<<QRect(x, y, w, h);
        m_painters[n].Init(n, img.copy(QRect(x, y, w, h)));
        connect(&m_threads[n], &QThread::started, &m_painters[n], &GAPainter::runSingle);
        connect(&m_painters[n], &GAPainter::sig_giveBackImage, m_imgStore, &ImageStore::setImage);
        m_painters[n].moveToThread(&m_threads[n]);
    }
}

GAPainterCollection::~GAPainterCollection()
{
    delete [] m_painters; m_painters = nullptr;
    delete m_imgStore; m_imgStore = nullptr;
}

void GAPainterCollection::run()
{
    for(int n = 0; n < m_painterNum; n++){
        m_threads[n].start();
    }
}


int ImageStore::_drawCount = 0;
int ImageStore::_saveCount = 0;

ImageStore::ImageStore(int sz, int h, int w, int overlap)
{
    m_SZ = sz;
    m_H = h;
    m_W = w;
    m_overLap = overlap;
    int d = sqrt(sz);
    imgSet = new std::vector<QImage>();
    for(int i = 0; i < sz; i++){
        this->imgSet->push_back(QImage(w/d, h/d, QImage::Format_RGBA8888));
        fitSet.push_back(0.f);
    }

    m_resultImage = QImage(m_W, m_H, QImage::Format_RGBA8888);
}

ImageStore::~ImageStore()
{
    if(imgSet != nullptr){
        delete imgSet;
        imgSet = nullptr;
    }
}

void ImageStore::draw()
{
    _drawCount++;
    //qDebug()<<"the draw was called and the count is "<<_drawCount;
    if(_drawCount < m_SZ+1){
        return;
    }
    qDebug()<<"draw the image!";
    _drawCount = 0;
    _saveCount++;

    QPainter p(&m_resultImage);
    int d = sqrt(m_SZ);
    int halfoverlap = m_overLap / 2;

    int h = m_H / d + m_overLap;
    int w = m_W / d + m_overLap;
    for(int n = 0; n < m_SZ; n++){
        int i = n / d;
        int j = n % d;

        int x = 0;
        int y = 0;
        if( i == 0)
            x = 0;
        else if(i == d-1)
            x = i*w - m_overLap - m_overLap;
        else
            x = i*w - m_overLap - halfoverlap;

        if( j == 0)
            y = 0;
        else if(j == d-1)
            y = j*h - m_overLap - m_overLap;
        else
            y = j*h - m_overLap - halfoverlap;

        p.drawImage(QPoint(x, y), imgSet->at(n));
    }

    float fit = 0.f;
    for(const auto& f: fitSet){
        fit += f;
    }
    fit /= fitSet.size();

    m_resultImage.save( QString("C:\\Users\\Liang\\Desktop\\GA\\Gen%1Yome%2.png").arg(_saveCount).arg(fit));
}

void ImageStore::setImage(int index, QImage img, float fit)
{
    imgSet->at(index) = img;
    fitSet.at(index) = fit;
    draw();
}
