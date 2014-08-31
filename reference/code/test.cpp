/******************************************************************************\
* Copyright (C) 2012-2013 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/
//#include <windows.h>//如果嫌刷屏太快打开这个定义1（一共2处）
#include <iostream>
#include "Leap.h"
using namespace Leap;

class SampleListener : public Listener {
public:
    virtual void onInit(const Controller&);
    virtual void onConnect(const Controller&);
    virtual void onDisconnect(const Controller&);
    virtual void onExit(const Controller&);
    virtual void onFrame(const Controller&);
    virtual void onFocusGained(const Controller&);
    virtual void onFocusLost(const Controller&);
};

void SampleListener::onInit(const Controller& controller) {
    std::cout << "Initialized" << std::endl;
}

//打开对所有手势的识别
void SampleListener::onConnect(const Controller& controller) {
    std::cout << "Connected" << std::endl;
    controller.enableGesture(Gesture::TYPE_CIRCLE);
    controller.enableGesture(Gesture::TYPE_KEY_TAP);
    controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
    controller.enableGesture(Gesture::TYPE_SWIPE);
}

void SampleListener::onDisconnect(const Controller& controller) {
    //Note: not dispatched when running in a debugger.
    std::cout << "Disconnected" << std::endl;
}

void SampleListener::onExit(const Controller& controller) {
    std::cout << "Exited" << std::endl;
}

//核心函数，当获取一帧数据时要做点什么事
void SampleListener::onFrame(const Controller& controller) {
    // 获取最新的一帧，并且返回一些基本信息
    const Frame frame = controller.frame();
    std::cout << "Frame id: " << frame.id()
        << ", timestamp: " << frame.timestamp()
        << ", hands: " << frame.hands().count()
        << ", fingers: " << frame.fingers().count()
        << ", tools: " << frame.tools().count()
        << ", gestures: " << frame.gestures().count() << std::endl;

    //非空，检测到手
    if (!frame.hands().isEmpty()) {
        // 得到第一只手[hands()返回HandList结构，可以向操作容器一样操作]
        const Hand hand = frame.hands()[0];

        // 检查下手是否有手指[握拳的话，是找不到手指的]
        const FingerList fingers = hand.fingers();
        if (!fingers.isEmpty()) {
            // 计算指尖尖端平均位置坐标
            Vector avgPos;
            for (int i = 0; i < fingers.count(); ++i) {
                avgPos += fingers[i].tipPosition();
            }
            avgPos /= (float)fingers.count();
            std::cout << "Hand has " << fingers.count()
                << " fingers, average finger tip position" << avgPos << std::endl;
        }

        // 获取手的球心半径和手掌的坐标
        std::cout << "Hand sphere radius: " << hand.sphereRadius()
            << " mm, palm position: " << hand.palmPosition() << std::endl;

        // 获取手的垂直向量（垂直手心向里）和方向（以手心开始，沿着手指指尖方向）
        const Vector normal = hand.palmNormal();
        const Vector direction = hand.direction();

        // 计算手的俯仰Pitch角度、平面Roll旋转角度和左右Yaw旋转角度
        std::cout << "Hand pitch: " << direction.pitch() * RAD_TO_DEG << " degrees, "
            << "roll: " << normal.roll() * RAD_TO_DEG << " degrees, "
            << "yaw: " << direction.yaw() * RAD_TO_DEG << " degrees" << std::endl;
    }
    /*他并没有使用一些函数rightmost和leftmost之类*/

    // 获取手势
    const GestureList gestures = frame.gestures();
    for (int g = 0; g < gestures.count(); ++g) {
        Gesture gesture = gestures[g];//这里最好去看下Gesture的定义，里面详细描述各种手势触发的阈值
        //编程技术比较厉害啊，都看不到Gesture的成员变量在哪里，完全被封装起来了，不是com就是dll封装的吧
        //我也很想学习这种完全把变量封装到看不见的方法，这样程序员会很省心（眼不见心为净）

        //注意下来使用了switch语句先判断它是否是这种类型手势，再进行类的重新构造
        switch (gesture.type()) {
        case Gesture::TYPE_CIRCLE:
            {
                CircleGesture circle = gesture;
                std::string clockwiseness;

                if (circle.pointable().direction().angleTo(circle.normal()) <= PI/4) {
                    clockwiseness = "clockwise";//顺时针旋转的话，circle.normal()指向外侧[右手坐标系，学过物理磁力线神马的一下子就会明白]
                } else {
                    clockwiseness = "counterclockwise";//逆时针，同上理解
                }

                // 计算和上一帧经过的角度
                float sweptAngle = 0;
                if (circle.state() != Gesture::STATE_START) {
                    //如果不是第一帧[第一帧肯定不能作为判断，只能作为初始化]
                    //frame(0)表示当前最新一帧，而frame(1)表示前一帧，以此类推
                    //对于id的使用十分简单明了，但很重要。这样表示只针对这个人的这个手指。
                    CircleGesture previousUpdate = CircleGesture(controller.frame(1).gesture(circle.id()));
                    //progress()表示手指旋转了几圈，如果是0.5表示旋转了半圈，如果是3表示已经旋转了3圈[计数功能很赞啊]
                    sweptAngle = (circle.progress() - previousUpdate.progress()) * 2 * PI;//一共旋转多少弧度[显示时他再次转换成角度]
                }
                std::cout << "Circle id: " << gesture.id()
                    << ", state: " << gesture.state()
                    << ", progress: " << circle.progress()
                    << ", radius: " << circle.radius()
                    << ", angle " << sweptAngle * RAD_TO_DEG
                    <<  ", " << clockwiseness << std::endl;
                break;
            }
        case Gesture::TYPE_SWIPE:
            {//相对旋转手势来说，这个以及下面几个简单很多
                SwipeGesture swipe = gesture;
                std::cout << "Swipe id: " << gesture.id()
                    << ", state: " << gesture.state()
                    << ", direction: " << swipe.direction()//这个可是能判断所有方向的，强大！我玩Kinect时只能识别几个特定的方向。
                    << ", speed: " << swipe.speed() << std::endl;
                break;
            }
        case Gesture::TYPE_KEY_TAP:
            {
                KeyTapGesture tap = gesture;
                std::cout << "Key Tap id: " << gesture.id()
                    << ", state: " << gesture.state()
                    << ", position: " << tap.position()
                    << ", direction: " << tap.direction()<< std::endl;
                break;
            }
        case Gesture::TYPE_SCREEN_TAP:
            {
                ScreenTapGesture screentap = gesture;
                std::cout << "Screen Tap id: " << gesture.id()
                    << ", state: " << gesture.state()
                    << ", position: " << screentap.position()
                    << ", direction: " << screentap.direction()<< std::endl;
                break;
            }
        default:
            std::cout << "Unknown gesture type." << std::endl;
            break;
        }
    }

    if (!frame.hands().isEmpty() || !gestures.isEmpty()) {
        std::cout << std::endl;
    }
    //  Sleep(1000);//如果嫌刷屏太快打开这个定义2（一共2处）
}

void SampleListener::onFocusGained(const Controller& controller) {
    std::cout << "Focus Gained" << std::endl;
}

void SampleListener::onFocusLost(const Controller& controller) {
    std::cout << "Focus Lost" << std::endl;
}

int main() {
    // 创建例子监听者和控制器
    SampleListener listener;
    Controller controller;

    // 让监听者获取控制器的数据并进行分析
    controller.addListener(listener);

    // 让程序一直运行，直到输入回车键才结束
    std::cout << "Press Enter to quit..." << std::endl;
    std::cin.get();

    // 移除监听者
    controller.removeListener(listener);

    return 0;
}