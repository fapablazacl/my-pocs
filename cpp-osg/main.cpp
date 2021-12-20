
#include <iostream>
#include <osg/Group>
#include <osg/Node>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>

#include <Windows.h>


LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam) {
	switch (nMsg) {
	case WM_CLOSE:
		::PostMessage(hWnd, WM_QUIT, 0, 0);
		return 0;

	case WM_PAINT:

		return 0;

	default:
		return DefWindowProc(hWnd, nMsg, wParam, lParam);
	}	
}


class Renderer {
public:
	Renderer() {
		mViewer.setCameraManipulator(new osgGA::TrackballManipulator());

		mModelNode = osgDB::readNodeFile("cow.osg");
		if (! mModelNode) {
			throw std::runtime_error("sadfads");
		}

		mRoot = new osg::Group();

		mRoot->addChild(mModelNode);

		mViewer.setSceneData(mRoot);
		mViewer.realize();

		if (! mViewer.isRealized()) {
			throw std::runtime_error("sadfads");
		}
	}


	void run() {
		while (!mViewer.done()) {
			mViewer.frame();
		}
	}


private:
	osgViewer::Viewer mViewer;
	osg::Node *mModelNode = nullptr;
	osg::Group *mRoot = nullptr;
};


int main() {
	try {
		//Renderer renderer;
		//renderer.run();

		WNDCLASS wc = {};
		wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
		wc.hInstance = ::GetModuleHandle(NULL);
		wc.lpszClassName = "Form1";
		wc.lpfnWndProc = WindowProc;
		wc.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		
		::RegisterClass(&wc);

		HWND hWnd = ::CreateWindow("Form1", "Form1", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL);
		::ShowWindow(hWnd, SW_NORMAL);

		MSG msg = {};

		while (::GetMessage(&msg, NULL, 0, 0) > 0) {
			if (msg.message == WM_QUIT) {
				break;
			} else {
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}

		if (hWnd) {
			::DestroyWindow(hWnd);
		}

		::UnregisterClass("Form1", wc.hInstance);

		return 0;

	} catch (const std::exception &exp) {
		std::cerr << exp.what() << std::endl;

		return 1;
	}
}
