/******************************************************************************\
* Copyright (C) 2012-2013 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/
//#include <windows.h>//�����ˢ��̫����������1��һ��2����
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

//�򿪶��������Ƶ�ʶ��
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

//���ĺ���������ȡһ֡����ʱҪ����ʲô��
void SampleListener::onFrame(const Controller& controller) {
    // ��ȡ���µ�һ֡�����ҷ���һЩ������Ϣ
    const Frame frame = controller.frame();
    std::cout << "Frame id: " << frame.id()
        << ", timestamp: " << frame.timestamp()
        << ", hands: " << frame.hands().count()
        << ", fingers: " << frame.fingers().count()
        << ", tools: " << frame.tools().count()
        << ", gestures: " << frame.gestures().count() << std::endl;

    //�ǿգ���⵽��
    if (!frame.hands().isEmpty()) {
        // �õ���һֻ��[hands()����HandList�ṹ���������������һ������]
        const Hand hand = frame.hands()[0];

        // ��������Ƿ�����ָ[��ȭ�Ļ������Ҳ�����ָ��]
        const FingerList fingers = hand.fingers();
        if (!fingers.isEmpty()) {
            // ����ָ����ƽ��λ������
            Vector avgPos;
            for (int i = 0; i < fingers.count(); ++i) {
                avgPos += fingers[i].tipPosition();
            }
            avgPos /= (float)fingers.count();
            std::cout << "Hand has " << fingers.count()
                << " fingers, average finger tip position" << avgPos << std::endl;
        }

        // ��ȡ�ֵ����İ뾶�����Ƶ�����
        std::cout << "Hand sphere radius: " << hand.sphereRadius()
            << " mm, palm position: " << hand.palmPosition() << std::endl;

        // ��ȡ�ֵĴ�ֱ��������ֱ��������ͷ��������Ŀ�ʼ��������ָָ�ⷽ��
        const Vector normal = hand.palmNormal();
        const Vector direction = hand.direction();

        // �����ֵĸ���Pitch�Ƕȡ�ƽ��Roll��ת�ǶȺ�����Yaw��ת�Ƕ�
        std::cout << "Hand pitch: " << direction.pitch() * RAD_TO_DEG << " degrees, "
            << "roll: " << normal.roll() * RAD_TO_DEG << " degrees, "
            << "yaw: " << direction.yaw() * RAD_TO_DEG << " degrees" << std::endl;
    }
    /*����û��ʹ��һЩ����rightmost��leftmost֮��*/

    // ��ȡ����
    const GestureList gestures = frame.gestures();
    for (int g = 0; g < gestures.count(); ++g) {
        Gesture gesture = gestures[g];//�������ȥ����Gesture�Ķ��壬������ϸ�����������ƴ�������ֵ
        //��̼����Ƚ�����������������Gesture�ĳ�Ա�����������ȫ����װ�����ˣ�����com����dll��װ�İ�
        //��Ҳ����ѧϰ������ȫ�ѱ�����װ���������ķ�������������Ա���ʡ�ģ��۲�����Ϊ����

        //ע������ʹ����switch������ж����Ƿ��������������ƣ��ٽ���������¹���
        switch (gesture.type()) {
        case Gesture::TYPE_CIRCLE:
            {
                CircleGesture circle = gesture;
                std::string clockwiseness;

                if (circle.pointable().direction().angleTo(circle.normal()) <= PI/4) {
                    clockwiseness = "clockwise";//˳ʱ����ת�Ļ���circle.normal()ָ�����[��������ϵ��ѧ����������������һ���Ӿͻ�����]
                } else {
                    clockwiseness = "counterclockwise";//��ʱ�룬ͬ�����
                }

                // �������һ֡�����ĽǶ�
                float sweptAngle = 0;
                if (circle.state() != Gesture::STATE_START) {
                    //������ǵ�һ֡[��һ֡�϶�������Ϊ�жϣ�ֻ����Ϊ��ʼ��]
                    //frame(0)��ʾ��ǰ����һ֡����frame(1)��ʾǰһ֡���Դ�����
                    //����id��ʹ��ʮ�ּ����ˣ�������Ҫ��������ʾֻ�������˵������ָ��
                    CircleGesture previousUpdate = CircleGesture(controller.frame(1).gesture(circle.id()));
                    //progress()��ʾ��ָ��ת�˼�Ȧ�������0.5��ʾ��ת�˰�Ȧ�������3��ʾ�Ѿ���ת��3Ȧ[�������ܺ��ް�]
                    sweptAngle = (circle.progress() - previousUpdate.progress()) * 2 * PI;//һ����ת���ٻ���[��ʾʱ���ٴ�ת���ɽǶ�]
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
            {//�����ת������˵������Լ����漸���򵥺ܶ�
                SwipeGesture swipe = gesture;
                std::cout << "Swipe id: " << gesture.id()
                    << ", state: " << gesture.state()
                    << ", direction: " << swipe.direction()//����������ж����з���ģ�ǿ������Kinectʱֻ��ʶ�𼸸��ض��ķ���
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
    //  Sleep(1000);//�����ˢ��̫����������2��һ��2����
}

void SampleListener::onFocusGained(const Controller& controller) {
    std::cout << "Focus Gained" << std::endl;
}

void SampleListener::onFocusLost(const Controller& controller) {
    std::cout << "Focus Lost" << std::endl;
}

int main() {
    // �������Ӽ����ߺͿ�����
    SampleListener listener;
    Controller controller;

    // �ü����߻�ȡ�����������ݲ����з���
    controller.addListener(listener);

    // �ó���һֱ���У�ֱ������س����Ž���
    std::cout << "Press Enter to quit..." << std::endl;
    std::cin.get();

    // �Ƴ�������
    controller.removeListener(listener);

    return 0;
}