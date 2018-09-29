#include "ofApp.h"
#include "ofxImGuiLite.hpp"

#include "peseudo_random.hpp"
// #include "SphericalRectangleSamplerCoordinate.hpp"
#include "SphericalRectangleSamplerCoordinate_Optimized.hpp"

static const int kSampleCount = 1024;

enum Sampler {
	Sampler_Random = 0,
	Sampler_BlueNoise,
};

//--------------------------------------------------------------
void ofApp::setup(){
	ofxImGuiLite::initialize();

	_camera.setNearClip(0.1);
	_camera.setFarClip(100.0);
	_camera.setDistance(5.0);

	std::ifstream infile(ofToDataPath("bluenoise.txt"));
	double x, y;
	while (infile >> x >> y)
	{
		_bluenoise.push_back(glm::dvec2(x, y));
	}
}

//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw(){
	ofEnableDepthTest();

	ofClear(0);
	_camera.begin();
	ofPushMatrix();
	ofRotateZDeg(90.0);
	ofSetColor(128);
	ofDrawGridPlane(1.0);
	ofPopMatrix();

	ofPushMatrix();
	ofDrawAxis(50);
	ofPopMatrix();

	ofSetCircleResolution(100);

	static int sampler_mode = Sampler_Random;
	static float exLen = 2.0f;
	static float eyLen = 1.0f;
	static float x0 = -1.0f;
	static float y0 = -0.5f;
	static float z0 = -2.0f;

	float x1 = x0 + exLen;
	float y1 = y0 + eyLen;

	glm::vec3 v[2][2];
	v[0][0] = glm::vec3(x0, y0, z0);
	v[0][1] = glm::vec3(x0, y1, z0);
	v[1][0] = glm::vec3(x1, y0, z0);
	v[1][1] = glm::vec3(x1, y1, z0);

	{
		ofSetColor(64);
		ofNoFill();
		ofDrawSphere(1.0);
		ofFill();

		auto draw_rectangle = [](glm::vec3 a, glm::vec3 b) {
			int N = 30;
			ofPolyline line;
			for (int i = 0; i < N; ++i) {
				float s = (float)i / (N - 1);
				glm::vec3 p = glm::mix(a, b, s);
				line.addVertex(glm::normalize(p));
			}
			line.draw();

			ofDrawLine(a, glm::vec3(0.0f));
			ofDrawLine(a, b);
		};
		ofSetColor(ofColor::white);
		draw_rectangle(v[0][0], v[1][0]);
		draw_rectangle(v[1][0], v[1][1]);
		draw_rectangle(v[1][1], v[0][1]);
		draw_rectangle(v[0][1], v[0][0]);
	}

	rt::Rectangle qp = rt::Rectangle(
		glm::dvec3(x0, y0, z0),
		glm::dvec3(exLen, 0.0, 0.0),
		glm::dvec3(0.0, eyLen, 0.0)
	);

	rt::SphericalRectangleSamplerCoordinate sampler(qp, glm::dvec3(0.0));

	{
		ofSetCircleResolution(10);

		rt::XoroshiroPlus128 random;
		
		for (int i = 0; i < kSampleCount; ++i) {
			double u = 0.0; 
			double v = 0.0;

			if (sampler_mode == Sampler_Random) {
				u = random.uniform();
				v = random.uniform();
			}
			else {
				if (i < _bluenoise.size()) {
					u = _bluenoise[i].x;
					v = _bluenoise[i].y;
				}
			}

			glm::dvec3 s = sampler.sample(u, v);

			ofSetColor(ofColor::orange);
			ofDrawSphere(s, 0.006f);
			ofDrawSphere(glm::normalize(s), 0.006f);
		}
	}

	ofDisableDepthTest();
	ofSetColor(255);

	ofxImGuiLite::ScopedImGui imgui;

	// camera control                                          for control clicked problem
	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || (ImGui::IsAnyWindowFocused() && ImGui::IsAnyMouseDown())) {
		_camera.disableMouseInput();
	}
	else {
		_camera.enableMouseInput();
	}

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiSetCond_Appearing);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiSetCond_Appearing);
	ImGui::SetNextWindowCollapsed(false, ImGuiSetCond_Appearing);
	ImGui::SetNextWindowBgAlpha(0.5f);

	ImGui::Begin("settings", nullptr);
	ImGui::Text("%.2f fps", ofGetFrameRate());

	const char *kSamplers[] = {
		"Random Sampler",
		"BlueNoise Sampler",
	};
	ImGui::Combo("Sampler", (int *)&sampler_mode, kSamplers, sizeof(kSamplers) / sizeof(kSamplers[0]));

	ImGui::SliderFloat("x0", &x0, -5.0f, 5.0f);
	ImGui::SliderFloat("y0", &y0, -5.0f, 5.0f);
	ImGui::SliderFloat("z0", &z0, -5.0f, 5.0f);

	ImGui::SliderFloat("exLen", &exLen, 0.0f, 5.0f);
	ImGui::SliderFloat("eyLen", &eyLen, 0.0f, 5.0f);

	if (ImGui::Button("Default Params")) {
		exLen = 2.0f;
		eyLen = 1.0f;
		x0 = -1.0f;
		y0 = -0.5f;
		z0 = -2.0f;
	}
	if (ImGui::Button("Trick Params")) {
		exLen = 5.0f;
		eyLen = 4.0f;
		x0 = 0.0f;
		y0 = -0.5f;
		z0 = -2.0f;
	}

	ImGui::End();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == 'f') {
		ofToggleFullscreen();
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
