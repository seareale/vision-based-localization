#ifndef KNUGLMODEL_H
#define KNUGLMODEL_H

#include <GL/gl3w.h>
#include <glm/glm.hpp>

#include <vector>

namespace SYE {
	class Shader;
	struct Vertex;
}

// 다양한 Opengl class가 늘어나는 것에 대비하여 공통 변수, 함수 등을 모아둠.
class KOglBase {
	private:
	protected:
		GLuint VAO, VBO;
		glm::mat4 mMatModel = glm::mat4(0.7f);	//초기 위치 - Model viewer matrix
		SYE::Shader *pShader;
		std::vector<SYE::Vertex> vertices;
		void setVAO();
	public:
		KOglBase() {};
		virtual ~KOglBase() {};

		virtual void render(glm::mat4 view, glm::mat4 proj) {};
};

class KOglGrid: public KOglBase {
	private:
	protected:
	public:
		KOglGrid(
				SYE::Shader *pShader
				, unsigned int rows = 10
				, unsigned int cols = 10
				, float unit = 0.1f
				, glm::vec3 color = glm::vec3(0.7f, 0.7f, 0.7f)
				);
		virtual ~KOglGrid();
		void render(glm::mat4 view, glm::mat4 proj);
};

////////////////////////////////////////////////////////////////////////////////////

enum eKOglObjectType {
	KObjTriangle,
	KObjRectangle,
	KObjCircle,
	KObEtc,
};

class KOglObject: public KOglBase {
	private:
		int type;
	protected:
	public:
		KOglObject();
		virtual ~KOglObject();

		void initialize(
				SYE::Shader *pShader
				, eKOglObjectType type
				, float unit = 0.1f
				, glm::vec3 color = glm::vec3(0.4f, 0.3f, 0.5f)
				);
		void render(glm::mat4 view, glm::mat4 proj);

};

#endif
