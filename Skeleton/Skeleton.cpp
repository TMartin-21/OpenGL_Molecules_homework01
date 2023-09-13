//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    :
// Neptun :
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

const int nTesselatedVertices = 100;

class Camera {
	vec2 Cam = vec2(0.0f, 0.0f);
public:
	void ShiftPlane(vec2 i) { Cam = Cam + i; }
	void SetDefault() { Cam = vec2(0, 0); }
	vec2 GetCam() { return Cam; }
};

Camera camera;

float Lorentz(vec3 p, vec3 q) { return p.x * q.x + p.y * q.y - p.z * q.z; }
float Distance(vec3 p, vec3 q) { return acoshf(-Lorentz(p, q)); }
vec3 Hyperbolic(vec2 p) { return vec3(p.x, p.y, sqrtf(p.x * p.x + p.y * p.y + 1)); }

class GPURenderer : GPUProgram {
	unsigned int vao, vbo;

	const char* const vertexSource = R"(
		#version 330				
		precision highp float;				
		layout(location = 0) in vec3 vp;
		void main() { gl_Position = vec4(vp.xy / (vp.z + 1), 0, 1); }
	)";

	const char* const fragmentSource = R"(
		#version 330			
		precision highp float;	 
		uniform vec3 color;		
		out vec4 outColor;		
		void main() { outColor = vec4(color, 1); }
	)";
public:
	GPURenderer() {
		glViewport(0, 0, windowWidth, windowHeight);
		glLineWidth(2.0f);
		create(vertexSource, fragmentSource, "fragmenColor");
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	}

	void Draw(int type, std::vector<vec3> vertices, vec3 color) {
		setUniform(color, "color");
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_DYNAMIC_DRAW);
		glDrawArrays(type, 0, vertices.size());
	}
};

GPURenderer* render;

int const MAX_CHARGE = 80;
int const MIN_CHARGE = 55;
int const MAX_MASS = 10;
float const INTENSITY_FACTOR = 1.0f / MAX_CHARGE;

class Atom {
public:
	int mass;
	int electricCharge;
	float intensity = 0;
	vec2 center;
	vec3 color;
	vec2 qForce = vec2(0, 0);
	vec2 vel = vec2(0, 0);
	float w = 0;
	float alpha = 0;
	vec2 F = vec2(0, 0);
	float M = 0;

	Atom(char charge) {
		center = vec2(2.0f * rand() / RAND_MAX - 1, 2.0f * rand() / RAND_MAX - 1);
		mass = rand() % MAX_MASS + 1; 
		electricCharge = rand() % (MAX_CHARGE - MIN_CHARGE + 1) + MIN_CHARGE;
		if (charge == 'N') {
			intensity = INTENSITY_FACTOR * electricCharge;
			color = vec3(0, 0, -intensity);
		}
		else if (charge == 'P') {
			electricCharge = -electricCharge;
			intensity = INTENSITY_FACTOR * electricCharge;
			color = vec3(intensity, 0, 0);
		}
	}

	void SetElectricCharge(int charge) {
		electricCharge = charge;
		intensity = INTENSITY_FACTOR * electricCharge;
		if (charge > 0) { color = vec3(intensity, 0, 0); }
		else { color = vec3(0, 0, -intensity); }
	}
};

class Binding {
	std::vector<vec3> vertices;
public:
	Binding(vec3 p, vec3 q) {
		for (int i = 0; i < nTesselatedVertices; i++) {
			float fi = i * 2.0f * M_PI / nTesselatedVertices;
			float dist = acoshf(-Lorentz(p, q));
			vec3 v = (q - p * coshf(dist)) / sinhf(dist);
			vec3 point = p * coshf(fi) + v * sinhf(fi);

			float pointDistance = acoshf(-Lorentz(point, p));
			if (pointDistance <= dist)
				vertices.push_back(point);
		}
	}

	void Draw() {
		render->Draw(GL_LINE_STRIP, vertices, vec3(1, 1, 1));
	}

	~Binding() {
		vertices.clear();
	}
};

class Molecule {
public:
	std::vector<std::pair<int, int>> edges;
	int n, neg, pos;
	std::vector<Atom*> atoms;
	std::vector<Binding*> bindings;

	vec2 centerOfGravity = vec2(0, 0);
	vec2 MOLECULE_F = 0;
	vec2 V;
	vec3 W;
	float theta = 0;
	float MOLECULE_M = 0;
	float MOLECULE_MASS = 0;

	Molecule() {
		neg = rand() % 7 + 1;
		pos = rand() % (8 - neg) + 1;
		n = neg + pos;
		edges = GenerateTree(n);

		for (auto edge : edges) {
			printf("\n(%d, %d)\n", edge.first, edge.second);
		}

		for (int i = 0; i < n; i++) {
			Atom* atom;
			if (i < neg) atom = new Atom('N');
			else atom = new Atom('P');
			atoms.push_back(atom);
		}

		for (auto edge : edges) {
			bindings.push_back(new Binding(Hyperbolic(atoms[edge.first]->center), Hyperbolic(atoms[edge.second]->center)));
		}

		for (auto atom : atoms) {
			MOLECULE_MASS += atom->mass;
		}
		Neutralize();
		SetPosition();
	}

	void Neutralize() {
		int sumCharge = 0;
		for (auto& atom : atoms) {
			sumCharge = sumCharge + atom->electricCharge;
		}
		if (sumCharge == 0) return;
		printf("SUM CHARGE BEFORE: %d\n", sumCharge);
		int dCharge = sumCharge / (int)atoms.size();
		int rCharge = sumCharge - (int)atoms.size() * dCharge;

		for (auto& atom : atoms) {
			atom->SetElectricCharge(atom->electricCharge - dCharge);
		}
		atoms[0]->SetElectricCharge(atoms[0]->electricCharge - rCharge);
		
		sumCharge = 0;
		for (auto& atom : atoms) {
			sumCharge = sumCharge + atom->electricCharge;
		}
		printf("SUM CHARGE AFTER: %d\n", sumCharge);
	}

	std::vector<std::pair<int, int>> GenerateTree(int n) {
		std::vector<std::pair<int, int>> edges;
		for (int i = 1; i < n; i++) {
			if (i == 1) {
				edges.push_back({ i - 1, i });
			}
			else {
				int n1 = rand() % i;
				edges.push_back({ n1, i });
			}
		}
		return edges;
	}

	void SetPosition() {
		for (int i = 0; i < atoms.size(); i++) {
			for (int j = i + 1; j < atoms.size(); j++) {
				vec2 vec_ij = atoms[j]->center - atoms[i]->center;
				if (length(vec_ij) < 1.2f) {
					atoms[j]->center = atoms[i]->center + 2.0f * vec_ij;
				} else if (length(vec_ij) > 2.0f) {
					atoms[j]->center = atoms[i]->center + 0.5f * vec_ij;
				}
			}
		}
	}

	void SetBindings() {
		bindings.clear();
		for (auto edge : edges) {
			bindings.push_back(new Binding(Hyperbolic(atoms[edge.first]->center), Hyperbolic(atoms[edge.second]->center)));
		}
	}

	void CenterOfGravity() {
		float X = 0, Y = 0;
		for (auto atom : atoms) {
			X += atom->center.x * atom->mass;
			Y += atom->center.y * atom->mass;
		}
		centerOfGravity = vec2(X, Y) / MOLECULE_MASS;
	}

	void Draw() {	
		for (auto binding : bindings) {
			SetBindings();
			binding->Draw();
		}
		for (auto atom : atoms) {
			vec3 hyper_center = Hyperbolic(atom->center);
			std::vector<vec3> vertices = { hyper_center };
			for (int i = 0; i < nTesselatedVertices + 1; i++) {
				float fi = i * 2.0f * M_PI / nTesselatedVertices;
				vec3 p = vec3(cosf(fi), sinf(fi), 0);
				p = p + hyper_center * Lorentz(hyper_center, p);
				p = p / sqrtf(Lorentz(p, p));
				vertices.push_back(hyper_center * coshf(0.08f) + p * sinhf(0.08f));
			}
			render->Draw(GL_TRIANGLE_FAN, vertices, atom->color);
		}
	}

	~Molecule() {
		for (int i = 0; i < n; i++) {
			delete atoms[i];
		}
		for (auto binding : bindings) {
			delete binding;
		}
	}
};


class System {
public:
	Molecule* molecule1, * molecule2;
	const float E = 170.0f;

	System() {
		molecule1 = new Molecule();
		molecule2 = new Molecule();
	}

	void CoulombForce(Atom *atom2, Molecule *molecule) {
		float rho = 2.0f;
		atom2->qForce = vec2(0, 0);
		atom2->F = vec2(0, 0);
		atom2->M = 0;

		std::vector<vec2> forces;
		for (auto atom1 : molecule->atoms) {
			vec2 vec21 = atom2->center - atom1->center;
			float d = length(vec21);
			float Q12 = atom1->electricCharge * atom2->electricCharge;
			vec2 e21 = normalize(vec21);
			vec2 F = (Q12 * e21) / (2 * M_PI * E * d + 0.0001f);
			forces.push_back(F);
		}
		for (auto force : forces)
			atom2->qForce = atom2->qForce + force;

		vec2 r = atom2->center - molecule->centerOfGravity;
		vec3 cr = cross(vec3(0, 0, atom2->w), r);
		atom2->qForce = atom2->qForce - rho * (atom2->vel + vec2(cr.x, cr.y));
		atom2->F = normalize(r) * dot(atom2->qForce, r);
		atom2->M = cross(r, atom2->qForce).z;
	}

	void Sumforce(Molecule* molecule) {
		vec2 SUM_F = vec2(0, 0);
		for (auto atom : molecule->atoms)
			SUM_F = SUM_F + atom->qForce;
		molecule->MOLECULE_F = SUM_F;
	}

	void Torque(Molecule* molecule) {
		molecule->MOLECULE_M = 0;
		molecule->theta = 0;
		for (auto atom : molecule->atoms) {
			vec2 r = atom->center - molecule->centerOfGravity;
			molecule->theta += atom->mass * dot(r, r);
			molecule->MOLECULE_M = molecule->MOLECULE_M + atom->M;
		}
	}

	void Simulation(Molecule* molecule1, Molecule* molecule2, float t) {
		molecule1->CenterOfGravity();
		float dt = t;
		if (dt > 0.01f) {
			dt = 0.01f;
		}
		vec2 MOLECULE_A = 0;
		float MOLECULE_B = 0;

		for (auto& atom : molecule1->atoms) {
			CoulombForce(atom, molecule2);
			atom->alpha = 0;
		}
		Sumforce(molecule1);
		Torque(molecule1);

		MOLECULE_A = molecule1->MOLECULE_F / molecule1->MOLECULE_MASS;
		MOLECULE_B = molecule1->MOLECULE_M / molecule1->theta;

		molecule1->V = molecule1->V + MOLECULE_A * dt;
		molecule1->W = molecule1->W + MOLECULE_B * dt;

		for (auto& atom : molecule1->atoms) {
			atom->vel = molecule1->V;
			atom->w = molecule1->W.x;
			molecule1->SetBindings();
		}	
	}

	void Move(Molecule* molecule, float t) {
		float dt = t;
		if (dt > 0.01f) {
			dt = 0.01f;
		}
		for (auto& atom : molecule->atoms) {
			vec2 ri = atom->center;
			ri = ri + atom->vel * dt;
			atom->center = ri;
			atom->alpha += atom->w * dt;
			Rotate(atom->alpha, molecule);
		}
	}

	void Rotate(float alpha, Molecule* molecule) {
		molecule->CenterOfGravity();
		mat4 Rotation = mat4(
			 cosf(alpha), sinf(alpha), 0, 0,
			-sinf(alpha), cosf(alpha), 0, 0,
			 0,			  0,		   1, 0,
			 0,			  0,		   0, 1
		);

		for (auto& atom : molecule->atoms) {
			vec2 r = atom->center - molecule->centerOfGravity;
			vec4 c = vec4(r.x, r.y, 0, 0) * Rotation;
			r = vec2(c.x, c.y) + molecule->centerOfGravity;
			atom->center = r;
		}
	}

	void Draw() {
		for (auto& atom : molecule2->atoms) {
			atom->center = atom->center - camera.GetCam();
		}
		for (auto& atom : molecule1->atoms) {
			atom->center = atom->center - camera.GetCam();
		}
		camera.SetDefault();
		molecule1->Draw();
		molecule2->Draw();
	}

	~System() {
		delete molecule1;
		delete molecule2;
	}
};

std::vector<vec3> circle;
System* sys;

void onInitialization() {
	long time = glutGet(GLUT_ELAPSED_TIME);
	srand(time);
	for (int i = 0; i < nTesselatedVertices; i++) {
		float fi = i * 2 * M_PI / nTesselatedVertices;
		vec3 point = Hyperbolic(1000 * vec2(cosf(fi), sinf(fi)));
		circle.push_back(point);
	}
	render = new GPURenderer();
	sys = new System();
}

void onDisplay() {
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	render->Draw(GL_TRIANGLE_FAN, circle, vec3(0.2f, 0.2f, 0.2f));
	sys->Draw();
	glutSwapBuffers();
}

void onKeyboard(unsigned char key, int pX, int pY) {
	switch (key) {
		case ' ': delete sys; sys = new System(); break;
		case 's': camera.ShiftPlane(vec2(-0.1f, 0)); break;
		case 'd': camera.ShiftPlane(vec2(0.1f, 0)); break;
		case 'e': camera.ShiftPlane(vec2(0, 0.1f)); break;
		case 'x': camera.ShiftPlane(vec2(0, -0.1f)); break;
	}
	glutPostRedisplay();
}

void onKeyboardUp(unsigned char key, int pX, int pY) {}
void onMouseMotion(int pX, int pY) {}
void onMouse(int button, int state, int pX, int pY) {}

void onIdle() {
	long time = glutGet(GLUT_ELAPSED_TIME);
	float sec = time / 100000.0f;
	sys->Simulation(sys->molecule1, sys->molecule2, sec);
	sys->Simulation(sys->molecule2, sys->molecule1, sec);
	sys->Move(sys->molecule1, sec);
	sys->Move(sys->molecule2, sec);
	glutPostRedisplay();
}