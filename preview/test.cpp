// path tracing test

#include <cmath>
#include <stdio.h>
#include <algorithm>
#pragma warning(disable: 4244 4305 4996)


#pragma region Windows

#ifndef UNICODE
#define UNICODE
#endif

#include <Windows.h>
#include <windowsx.h>
#include <tchar.h>

#define WIN_NAME "UI"
#define WinW_Padding 100
#define WinH_Padding 100
#define WinW_Default 640
#define WinH_Default 400
#define WinW_Min 300
#define WinH_Min 200
#define WinW_Max 1920
#define WinH_Max 1080

void Init();  // only use this function to initialize variables (or test)
void render();
void WindowResize(int _oldW, int _oldH, int _W, int _H);
void WindowClose();
void MouseMove(int _X, int _Y);
void MouseWheel(int _DELTA);
void MouseDownL(int _X, int _Y);
void MouseUpL(int _X, int _Y);
void MouseDownR(int _X, int _Y);
void MouseUpR(int _X, int _Y);
void KeyDown(WPARAM _KEY);
void KeyUp(WPARAM _KEY);

HWND _HWND; int _WIN_W, _WIN_H;
HBITMAP _HIMG; COLORREF *_WINIMG;
#define Canvas(x,y) _WINIMG[(y)*_WIN_W+(x)]
#define setColor(x,y,col) do{if((x)>=0&&(x)<_WIN_W&&(y)>=0&&(y)<_WIN_H)Canvas(x,y)=col;}while(0)

double _DEPTHBUF[WinW_Max][WinH_Max];  // how you use this depends on you



// Win32 Entry

bool Render_Needed = true;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
#define _RDBK { if (!Render_Needed) break; HDC hdc = GetDC(hWnd), HImgMem = CreateCompatibleDC(hdc); HBITMAP hbmOld = (HBITMAP)SelectObject(HImgMem, _HIMG); render(); BitBlt(hdc, 0, 0, _WIN_W, _WIN_H, HImgMem, 0, 0, SRCCOPY); SelectObject(HImgMem, hbmOld), DeleteDC(HImgMem), DeleteDC(hdc); Render_Needed = false; break; }
	switch (message) {
	case WM_CREATE: { if (!_HWND) Init(); break; }
	case WM_CLOSE: { WindowClose(); DestroyWindow(hWnd); return 0; }
	case WM_DESTROY: { PostQuitMessage(0); return 0; }
	case WM_MOVE:; case WM_SIZE: {
		RECT Client; GetClientRect(hWnd, &Client); WindowResize(_WIN_W, _WIN_H, Client.right, Client.bottom); _WIN_W = Client.right, _WIN_H = Client.bottom;
		BITMAPINFO bmi; bmi.bmiHeader.biSize = sizeof(BITMAPINFO), bmi.bmiHeader.biWidth = Client.right, bmi.bmiHeader.biHeight = Client.bottom, bmi.bmiHeader.biPlanes = 1, bmi.bmiHeader.biBitCount = 32; bmi.bmiHeader.biCompression = BI_RGB, bmi.bmiHeader.biSizeImage = 0, bmi.bmiHeader.biXPelsPerMeter = bmi.bmiHeader.biYPelsPerMeter = 0, bmi.bmiHeader.biClrUsed = bmi.bmiHeader.biClrImportant = 0; bmi.bmiColors[0].rgbBlue = bmi.bmiColors[0].rgbGreen = bmi.bmiColors[0].rgbRed = bmi.bmiColors[0].rgbReserved = 0;
		if (_HIMG != NULL) DeleteObject(_HIMG); HDC hdc = GetDC(hWnd); _HIMG = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&_WINIMG, NULL, 0); DeleteDC(hdc); _RDBK
	}
	case WM_GETMINMAXINFO: { LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam; lpMMI->ptMinTrackSize.x = WinW_Min, lpMMI->ptMinTrackSize.y = WinH_Min, lpMMI->ptMaxTrackSize.x = WinW_Max, lpMMI->ptMaxTrackSize.y = WinH_Max; break; }
	case WM_PAINT: { PAINTSTRUCT ps; HDC hdc = BeginPaint(hWnd, &ps), HMem = CreateCompatibleDC(hdc); HBITMAP hbmOld = (HBITMAP)SelectObject(HMem, _HIMG); BitBlt(hdc, 0, 0, _WIN_W, _WIN_H, HMem, 0, 0, SRCCOPY); SelectObject(HMem, hbmOld); EndPaint(hWnd, &ps); DeleteDC(HMem), DeleteDC(hdc); break; }
#define _USER_FUNC_PARAMS GET_X_LPARAM(lParam), _WIN_H - 1 - GET_Y_LPARAM(lParam)
	case WM_MOUSEMOVE: { MouseMove(_USER_FUNC_PARAMS); _RDBK }
	case WM_MOUSEWHEEL: { MouseWheel(GET_WHEEL_DELTA_WPARAM(wParam)); _RDBK }
	case WM_LBUTTONDOWN: { SetCapture(hWnd); MouseDownL(_USER_FUNC_PARAMS); _RDBK }
	case WM_LBUTTONUP: { ReleaseCapture(); MouseUpL(_USER_FUNC_PARAMS); _RDBK }
	case WM_RBUTTONDOWN: { MouseDownR(_USER_FUNC_PARAMS); _RDBK }
	case WM_RBUTTONUP: { MouseUpR(_USER_FUNC_PARAMS); _RDBK }
	case WM_SYSKEYDOWN:; case WM_KEYDOWN: { if (wParam >= 0x08) KeyDown(wParam); _RDBK }
	case WM_SYSKEYUP:; case WM_KEYUP: { if (wParam >= 0x08) KeyUp(wParam); _RDBK }
	} return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	WNDCLASSEX wc; wc.cbSize = sizeof(WNDCLASSEX), wc.style = 0, wc.lpfnWndProc = WndProc, wc.cbClsExtra = wc.cbWndExtra = 0, wc.hInstance = hInstance; wc.hIcon = wc.hIconSm = 0, wc.hCursor = LoadCursor(NULL, IDC_ARROW), wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0)), wc.lpszMenuName = NULL, wc.lpszClassName = _T(WIN_NAME);
	if (!RegisterClassEx(&wc)) return -1;
	_HWND = CreateWindow(_T(WIN_NAME), _T(WIN_NAME), WS_OVERLAPPEDWINDOW, WinW_Padding, WinH_Padding, WinW_Default, WinH_Default, NULL, NULL, hInstance, NULL);
	ShowWindow(_HWND, nCmdShow); UpdateWindow(_HWND);
	MSG message; while (GetMessage(&message, 0, 0, 0)) { TranslateMessage(&message); DispatchMessage(&message); } return (int)message.wParam;
}

#pragma endregion







#include "numerical/geometry.h"

const vec3 vec0(0, 0, 0), veci(1, 0, 0), vecj(0, 1, 0), veck(0, 0, 1);
#define SCRCTR vec2(0.5*_WIN_W,0.5*_WIN_H)



#pragma region Global Variables and Functions

// viewport
vec3 Center(0.0, 0.0, 0.0);  // view center in world coordinate
double rz = -0.8, rx = 0.3, ry = 0.0, dist = 60.0, Unit = 20.0;  // yaw, pitch, row, camera distance, scale to screen

// window parameters
char text[64];	// window title
vec3 CamP, ScrO, ScrA, ScrB;  // camera and screen
auto scrPos = [](vec2 pixel) { return ScrO + (pixel.x / _WIN_W)*ScrA + (pixel.y / _WIN_H)*ScrB; };
auto scrDir = [](vec2 pixel) { return normalize(scrPos(pixel) - CamP); };

// user parameters
vec2 Cursor = vec2(0, 0), clickCursor;  // current cursor and cursor position when mouse down
bool mouse_down = false;
bool Ctrl = false, Shift = false, Alt = false;  // these variables are shared by both windows


// projection
void getRay(vec2 Cursor, vec3 &p, vec3 &d) {
	p = CamP;
	d = normalize(ScrO + (Cursor.x / _WIN_W)*ScrA + (Cursor.y / _WIN_H)*ScrB - CamP);
}
void getScreen(vec3 &P, vec3 &O, vec3 &A, vec3 &B) {  // O+uA+vB
	double cx = cos(rx), sx = sin(rx), cz = cos(rz), sz = sin(rz);
	vec3 u(-sz, cz, 0), v(-cz * sx, -sz * sx, cx), w(cz * cx, sz * cx, sx);
	mat3 Y = axis_angle(w, -ry); u = Y * u, v = Y * v;
	u *= 0.5*_WIN_W / Unit, v *= 0.5*_WIN_H / Unit, w *= dist;
	P = Center + w;
	O = Center - (u + v), A = u * 2.0, B = v * 2.0;
}


#pragma endregion







// ============================================ Rendering ============================================

#include <vector>
#include "raytracing/bvh.h"


int Trig_N;
BVH* BVH_R = 0;



#include <chrono>
typedef std::chrono::high_resolution_clock NTime;
typedef std::chrono::duration<double> fsec;

COLORREF toCOLORREF(const vec3 &col) {
	COLORREF C; byte* c = (byte*)&C;
	c[2] = (byte)(255 * clamp(col.x, 0., 1.));
	c[1] = (byte)(255 * clamp(col.y, 0., 1.));
	c[0] = (byte)(255 * clamp(col.z, 0., 1.));
	return C;
}


#define MultiThread 1
#include <thread>

#if MultiThread
void Render_Exec(void(*task)(int, int, int, bool*), int Max) {
	const int MAX_THREADS = std::thread::hardware_concurrency();
	bool* fn = new bool[MAX_THREADS];
	std::thread** T = new std::thread*[MAX_THREADS];
	for (int i = 0; i < MAX_THREADS; i++) {
		fn[i] = false;
		T[i] = new std::thread(task, i, Max, MAX_THREADS, &fn[i]);
	}
	int count; do {
		count = 0;
		for (int i = 0; i < MAX_THREADS; i++) count += fn[i];
	} while (count < MAX_THREADS);
	//for (int i = 0; i < MAX_THREADS; i++) delete T[i];
	delete fn; delete T;
}
#else
void Render_Exec(void(*task)(int, int, int, bool*), int Max) {
	task(0, Max, 1, NULL);
}
#endif




// intersection between ray and sphere
bool intersectSphere(vec3 O, double r, vec3 ro, vec3 rd, double &t, vec3 &n) {
	ro -= O;
	double b = -dot(ro, rd), c = dot(ro, ro) - r * r;
	double delta = b * b - c;
	if (delta < 0.0) return false;
	delta = sqrt(delta);
	double t1 = b - delta, t2 = b + delta;
	if (t1 > t2) std::swap(t1, t2);
	if (t1 > t || t2 < 0.) return false;
	t = t1 > 0. ? t1 : t2;
	n = normalize(ro + rd * t);
	return true;
}

// test scene
bool intersectScene_test(BVH* R, vec3 ro, vec3 rd, double &t, vec3 &n) {
	return intersectSphere(vec3(0, 0, 1.1), 1.0, ro, rd, t, n);
}



#include "brdf.h"
double* plane_brdf = NULL;
double* cup_brdf = NULL;

vec3 colorBuffer[WinW_Max][WinH_Max];
int SPP = 0;  // sample per pixel

// dome light
vec3 calcCol(vec3 ro, vec3 rd, uint32_t &seed) {

	vec3 m_col = vec3(1.), col;

	// "recursive" ray-tracing
	for (int iter = 0; iter < 20; iter++) {
		vec3 n, min_n;
		ro += 1e-6*rd;  // alternate of t>1e-6

		int intersect_id = -1;  // which object the ray hits

		// intersect plane
		double min_t = -ro.z / rd.z;
		if (min_t > 0.) {
			min_n = vec3(0, 0, ro.z > 0. ? 1 : -1.);
			intersect_id = 0;
		}
		else {
			min_t = INFINITY;
		}

		// intersect cup
		double t = min_t;
		if (intersectScene(BVH_R, ro, rd, t, n)) {
			min_t = t, min_n = n;
			intersect_id = 1;
		}

		// color calculation and update ray

		// background
		if (intersect_id == -1) {
			// calculate incoming light
			vec3 col = vec3(50.*pow(max(dot(rd, normalize(vec3(0.1, 0.3, 1))), 0.), 5.));
			return m_col * col;
		}

		//min_n = dot(rd, min_n) < 0. ? min_n : -min_n;

		vec3 wi = -rd;

		// plane
		if (intersect_id == 0) {
			// ray calculation
			vec3 wi = -rd;
			vec3 wo = randdir_cosWeighted(min_n, seed);
			// color calculation
			vec2 p = ro.xy() + min_t * rd.xy();
			col = int(floor(p.x) + floor(p.y)) & 1 ? vec3(0.8) : vec3(0.6);
			col = PI * getBRDF(plane_brdf, wi, wo, min_n);
			m_col *= col;
			// update ray
			ro = ro + rd * min_t;
			rd = wo;
		}

		if (intersect_id == 1) {
			// ray calculation
			vec3 wi = -rd;
			vec3 wo = randdir_cosWeighted(min_n, seed);
			// color calculation
			col = PI * getBRDF(cup_brdf, wi, wo, min_n);
			m_col *= col;
			// update ray
			ro = ro + rd * min_t;
			rd = wo;
		}
	}
	return m_col;
}



void render_RT_BVH() {
	static uint32_t call_time = 0;
	call_time = lcg_next(call_time);

	SPP++;

	Render_Exec([](int beg, int end, int step, bool* sig) {
		const int WIN_SIZE = _WIN_W * _WIN_H;
		for (int k = beg; k < end; k += step) {
			int i = k % _WIN_W, j = k / _WIN_W;
			// brute force Monte-Carlo sampling
			const int N = 1;
			vec3 col(0.);
			for (int u = 0; u < N; u++) {
				uint32_t seed = hashu(u*WIN_SIZE + k + call_time);
				vec3 d = scrDir(vec2(i + rand01(seed), j + rand01(seed)));
				col += calcCol(CamP, d, seed);
			}
			if (SPP == 1) colorBuffer[i][j] = vec3(0.);
			colorBuffer[i][j] += col / N;
			Canvas(i, j) = toCOLORREF(colorBuffer[i][j] / SPP);
		}
		if (sig) *sig = true;
	}, _WIN_W*_WIN_H);
}



void render() {
	auto t0 = NTime::now();
	// initialize window
	for (int i = 0, l = _WIN_W * _WIN_H; i < l; i++) _WINIMG[i] = 0;
	for (int i = 0; i < _WIN_W; i++) for (int j = 0; j < _WIN_H; j++) _DEPTHBUF[i][j] = INFINITY;
	getScreen(CamP, ScrO, ScrA, ScrB);
	printf("W=%d,H=%d; CamP=vec3(%lf,%lf,%lf),ScrO=vec3(%lf,%lf,%lf),ScrA=vec3(%lf,%lf,%lf),ScrB=vec3(%lf,%lf,%lf);\n",
		_WIN_W, _WIN_H, CamP.x, CamP.y, CamP.z, ScrO.x, ScrO.y, ScrO.z, ScrA.x, ScrA.y, ScrA.z, ScrB.x, ScrB.y, ScrB.z);


	render_RT_BVH();


	double t = fsec(NTime::now() - t0).count();
	sprintf(text, "[%d×%d, %d]  %dspp  %.1fms (%.1ffps)\n", _WIN_W, _WIN_H, Trig_N, SPP, 1000.0*t, 1. / t);
	SetWindowTextA(_HWND, text);
}


// ============================================== User ==============================================



#include "modeling/parametric/surfaces.h"
#include "triangulate/parametric_surface_adaptive_dist.h"

bool inited = false;
void Init() {
	if (inited) return; inited = true;

	std::vector<BVH_Triangle*> BT;
	readBinarySTL("D:\\.stl", BT);
	Center = vec3(0, 0, 5);

	BVH_R = new BVH;
	vec3 Min(INFINITY), Max(-INFINITY);
	constructBVH(BVH_R, BT, Min, Max);

	if (!plane_brdf) {
		BRDFDatabase::read_brdf("D:\\Homework\\AVI3M\\AVI3M-CPT\\preview\\BRDFDatabase\\brdfs\\alum-bronze.binary", plane_brdf);
		for (double a = 0.; a < PI; a += 0.01*PI) {
			vec3 wi = vec3(1e-8, 1e-9, 1);
			vec3 wo = vec3(cos(a), 0, sin(a));
			vec3 fr = getBRDF(plane_brdf, wi, wo, vec3(0, 0, 1));
			printf("(%.3lg,%.3lg),", a, fr.x + fr.y + fr.z);
		}
		//exit(0);
	}
	if (!cup_brdf)
		BRDFDatabase::read_brdf("D:\\Homework\\AVI3M\\AVI3M-CPT\\preview\\BRDFDatabase\\brdfs\\pvc.binary", cup_brdf);
}


void keyDownShared(WPARAM _KEY) {
	if (_KEY == VK_CONTROL) Ctrl = true;
	else if (_KEY == VK_SHIFT) Shift = true;
	else if (_KEY == VK_MENU) Alt = true;
}
void keyUpShared(WPARAM _KEY) {
	if (_KEY == VK_CONTROL) Ctrl = false;
	else if (_KEY == VK_SHIFT) Shift = false;
	else if (_KEY == VK_MENU) Alt = false;
}

void WindowResize(int _oldW, int _oldH, int _W, int _H) {
	if (_W*_H == 0 || _oldW * _oldH == 0) return;  // window is minimized
	double pw = _oldW, ph = _oldH, w = _W, h = _H;
	double s = sqrt((w * h) / (pw * ph));
	Unit *= s, dist /= s;
	Render_Needed = true;
	SPP = 0;
}
void WindowClose() {}

void MouseWheel(int _DELTA) {
	Render_Needed = true;
	if (_DELTA) SPP = 0;
	if (Ctrl) Center.z += 0.1 * _DELTA / Unit;
	else if (Shift) {
		dist *= exp(-0.001*_DELTA);
	}
	else {
		double s = exp(0.001*_DELTA);
		double D = length(vec2(_WIN_W, _WIN_H)), Max = 1000.0*D, Min = 0.001*D;
		if (Unit * s > Max) s = Max / Unit; else if (Unit * s < Min) s = Min / Unit;
		Unit *= s, dist /= s;
	}
}
void MouseDownL(int _X, int _Y) {
	clickCursor = Cursor = vec2(_X, _Y);
	mouse_down = true;
	Render_Needed = true;
}
void MouseMove(int _X, int _Y) {
	vec2 P0 = Cursor, P = vec2(_X, _Y), D = P - P0;
	Cursor = P;

	Render_Needed = true;

	if (mouse_down) {
		SPP = 0;
		if (Ctrl) {
			vec3 d = scrDir(P0);
			vec3 p = CamP.z / d.z * d;
			d = scrDir(P);
			vec3 q = CamP.z / d.z * d;
			Center += q - p;
		}
		else if (Shift) {
			ry += 0.005*D.y;
		}
		else {
			vec2 d = 0.01*D;
			rz -= cos(ry)*d.x + sin(ry)*d.y, rx -= -sin(ry)*d.x + cos(ry)*d.y;  // doesn't work very well
			//rz -= d.x, rx -= d.y;
		}
	}

}
void MouseUpL(int _X, int _Y) {
	Cursor = vec2(_X, _Y);
	bool moved = (int)length(clickCursor - Cursor) != 0;
	mouse_down = false;
	Render_Needed = true;
}
void MouseDownR(int _X, int _Y) {
	Cursor = vec2(_X, _Y);
	Render_Needed = true;
}
void MouseUpR(int _X, int _Y) {
	Cursor = vec2(_X, _Y);
}
void KeyDown(WPARAM _KEY) {
	keyDownShared(_KEY);
}
void KeyUp(WPARAM _KEY) {
	keyUpShared(_KEY);
}
