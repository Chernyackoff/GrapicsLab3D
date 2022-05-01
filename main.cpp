#include <QApplication>
#include <QPushButton>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QLabel>
#include <QGraphicsView>
#include <QVector3D>
#include <QKeyEvent>
#include <QGraphicsLineItem>


const double projectionAngle = 30 * std::numbers::pi / 180;
const int delta_move = 10;
const double rotation_angle = 15 * std::numbers::pi / 180;
const std::array<QVector3D, 6> default_points = {QVector3D(40, 30, 50),
                                                 {30, 50, 50},
                                                 {50, 50, 50},
                                                 {40, 30, 30},
                                                 {30, 50, 30},
                                                 {50, 50, 30}};

class Prism {
public:
    Prism(QGraphicsScene *scene_, const std::array<QVector3D, 6> &points) : scene(scene_), points3D(points), points2D(),
                                                                            lines(), center(), isFlat(true) {
        double sumX = 0, sumY = 0, sumZ = 0;
        for (auto point: points3D) {
            sumX += point.x();
            sumY += point.y();
            sumZ += point.z();
        }

        project();
        //lines init
        for (int i = 0; i < 3; i++) {
            lines[i] = scene->addLine({points2D[i], points2D[(i + 1) % 3]});
            lines[i + 3] = scene->addLine({points2D[i], points2D[i + 3]});
            lines[i + 6] = scene->addLine({points2D[i + 3], points2D[3 + (i + 1) % 3]});
        }

        center.setX(sumX / 6);
        center.setY(sumY / 6);
        center.setZ(sumZ / 6);
    }

    void move(double dx, double dy, double dz) {
        for (auto &point: points3D) {
            point.setX(point.x() + dx);
            point.setY(point.y() + dy);
            point.setZ(point.z() + dz);
        }
        center.setX(center.x() + dx);
        center.setY(center.y() + dy);
        center.setZ(center.z() + dz);
        update();
    }

    void rotateX(double a) {
        for (auto &point: points3D) {
            auto dz = point.z() - center.z(), dy = point.y() - center.y();
            double sz, sy;
            sy = dy * cos(a) - dz * sin(a);
            sz = dy * sin(a) + dz * cos(a);
            point.setZ(sz + center.z());
            point.setY(sy + center.y());
        }
        update();
    };

    void rotateY(double a) {
        for (auto &point: points3D) {
            auto dz = point.z() - center.z(), dx = point.x() - center.x();
            double sx, sz;
            sx = dx * cos(a) + dz * sin(a);
            sz = -dx * sin(a) + dz * cos(a);
            point.setZ(sz + center.z());
            point.setX(sx + center.x());
        }
        update();
    };

    void rotateZ(double a) {
        for (auto &point: points3D) {
            auto dx = point.x() - center.x(), dy = point.y() - center.y();
            double sx, sy;
            sx = dx * cos(a) - dy * sin(a);
            sy = dx * sin(a) + dy * cos(a);
            point.setX(sx + center.x());
            point.setY(sy + center.y());
        }
        update();
    };

    void resize(double delta) {
        for (auto &point: points3D) {
            auto dx = point.x() - center.x(), dy = point.y() - center.y(), dz = point.z() - center.z();
            double sx, sy, sz;
            sx = dx * delta;
            sy = dy * delta;
            sz = dz * delta;
            point.setX(sx + center.x());
            point.setY(sy + center.y());
            point.setZ(sz + center.z());
        }
        update();
    }

    void changeProjection() {
        isFlat = !isFlat;
        update();
    }
    double getCenterZPos(){
        return center.z();
    }

private:
    bool isFlat;
    QGraphicsScene *scene;
    QVector3D center;
    std::array<QVector3D, 6> points3D;
    std::array<QPointF, 6> points2D;
    std::array<QGraphicsLineItem *, 9> lines;

    void project() {
        isFlat ? projection_flat() : projection_volume();
    }

    void projection_flat() {
        for (int i = 0; i < 6; i++) {
            points2D[i] = points3D[i].toPointF();
        }
    };

    void projection_volume() {
        for(int i = 0; i < 6; i++){
            double xp, yp;
            xp = points3D[i].x() + points3D[i].z()*cos(projectionAngle); // Jean Cavalier projection
            yp = points3D[i].y() + points3D[i].z()*sin(projectionAngle);
            points2D[i].setX(xp);
            points2D[i].setY(yp);
        }
    };

    void update_lines(){
        for (int i = 0; i < 3; i++) {
            lines[i]->setLine({points2D[i], points2D[(i + 1) % 3]});
            lines[i + 3]->setLine({points2D[i], points2D[i + 3]});
            lines[i + 6]->setLine({points2D[i + 3], points2D[3 + (i + 1) % 3]});
        }
    };

    void update(){
        project();
        update_lines();
    };

};

class MainWindow : public QMainWindow {
public:
    MainWindow() : QMainWindow(), scene(), view(&scene) {
        resize(800, 600);
        auto central = new QWidget(this);
        auto layout = new QHBoxLayout(central);
        central->setLayout(layout);
        setCentralWidget(central);

        auto vLayout = new QVBoxLayout(central);
        auto label = new QLabel("Controls:\n1. Movement: WASDVB\n"
                                "2. Rotation Z: QE\n3. Rotation Y: RF\n"
                                "4. Rotation X: TG\n"
                                "5. Scale: ZC\n"
                                "6. Change projection: Space",
                                central);

        vLayout->addWidget(label);
        layout->addLayout(vLayout);
        layout->addWidget(&view);

        scene.setParent(central);
        view.setParent(central);

        scene.setSceneRect(0, 0, 550, 550);
        view.setSceneRect(0, 0, 550, 550);

        prism = new Prism(&scene, default_points);

        labelZ = new QLabel("Center Z: 40", central);
        vLayout->addWidget(labelZ);

    }

    void keyPressEvent(QKeyEvent *event) override {
        switch (event->key()) {
            case Qt::Key_W:
                prism->move(0, -delta_move, 0);
                break;
            case Qt::Key_S:
                prism->move(0, delta_move, 0);
                break;
            case Qt::Key_A:
                prism->move(-delta_move, 0, 0);
                break;
            case Qt::Key_D:
                prism->move(delta_move, 0, 0);
                break;
            case Qt::Key_V:
                prism->move(0, 0, -delta_move);
                updateLabel();
                break;
            case Qt::Key_B:
                prism->move(0, 0, delta_move);
                updateLabel();
                break;
            case Qt::Key_Q:
                prism->rotateZ(-rotation_angle);
                break;
            case Qt::Key_E:
                prism->rotateZ(rotation_angle);
                break;
            case Qt::Key_R:
                prism->rotateY(-rotation_angle);
                break;
            case Qt::Key_F:
                prism->rotateY(rotation_angle);
                break;
            case Qt::Key_T:
                prism->rotateX(-rotation_angle);
                break;
            case Qt::Key_G:
                prism->rotateX(rotation_angle);
                break;
            case Qt::Key_Z:
                prism->resize(0.7);
                break;
            case Qt::Key_C:
                prism->resize(1.3);
                break;
            case Qt::Key_Space:
                prism->changeProjection();
                break;
        }
  }

private:
    QLabel* labelZ;
    QGraphicsScene scene;
    QGraphicsView view;
    Prism *prism;

    void updateLabel(){
       auto s = QString("Center Z: %1").arg(prism->getCenterZPos());
       labelZ->setText(s);
    }
};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow window;
    window.show();
    return QApplication::exec();
}
