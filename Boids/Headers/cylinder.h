#pragma once
#define _USE_MATH_DEFINES

#include <vector>

const unsigned int MIN_LONGITUDE = 3;
const unsigned int MIN_LATITUDE = 1;

class Cylinder
{
public:

	Cylinder(float baseRadius = 1.0f, float topRadius = 1.0f, float height = 1.0f, unsigned int longitude = 30, unsigned int latitude = 30, bool smooth = true) {
		this->set(baseRadius, topRadius, height, longitude, latitude);
	}

	float getBaseRadius() const {
		return this->BaseRadius;
	}
	
	float getTopRadius() const {
		return this->TopRadius;
	}
	
	float getHeight() const {
		return this->Height;
	}
	
	unsigned int getLongitude() const {
		return this->Longitude;
	}
	
	unsigned int getLatitude() const {
		return this->Latitude;
	}
	
	void set(float baseRadius, float topRadius, float height, unsigned int longitude, unsigned int latitude) {
		this->BaseRadius = baseRadius;
		this->TopRadius = topRadius;
		this->Height = height;

		this->Longitude = longitude;
		if (longitude < MIN_LONGITUDE) {
			this->Longitude = MIN_LONGITUDE;
		}

		this->Latitude = latitude;
		if (longitude < MIN_LONGITUDE) {
			this->Latitude = MIN_LATITUDE;
		}

		this->generateUnitCircleVertices();

		this->generateVertices();

		// this->showInfo();
	}

	void setBaseRadius(float radius) {
		if (this->BaseRadius != radius) {
			this->set(radius, this->TopRadius, this->Height, this->Longitude, this->Latitude);
		}
	}
	
	void setTopRadius(float radius) {
		if (this->TopRadius != radius) {
			this->set(this->BaseRadius, radius, this->Height, this->Longitude, this->Latitude);
		}
	}
	
	void setHeight(float height) {
		if (this->Height != height) {
			this->set(this->BaseRadius, this->TopRadius, height, this->Longitude, this->Latitude);
		}
	}
	
	void setLongitude(unsigned int longitude) {
		if (this->Longitude != longitude) {
			this->set(this->BaseRadius, this->TopRadius, this->Height, longitude, this->Latitude);
		}
	}
	
	void setLatitude(unsigned int latitude) {
		if (this->Latitude != latitude) {
			this->set(this->BaseRadius, this->TopRadius, this->Height, this->Longitude, latitude);
		}
	}

	unsigned int getVertexCount() const {
		return (unsigned int)this->Vertices.size() / 8;
	}

	unsigned int getPositionCount() const {
		return (unsigned int)this->Position.size() / 3;
	}

	unsigned int getNormalCount() const {
		return (unsigned int)this->Normals.size() / 3;
	}

	unsigned int getTexCoordCount() const {
		return (unsigned int)this->TexCoords.size() / 2;
	}

	unsigned int getIndexCount() const {
		return (unsigned int)this->Indices.size();
	}

	unsigned int getTriangleCount() const {
		return (unsigned int)this->getIndexCount() / 3;
	}

	unsigned int getVertexSize() const {
		return (unsigned int)this->Vertices.size() * sizeof(float);
	}

	unsigned int getPositionSize() const {
		return (unsigned int)this->Position.size() * sizeof(float);
	}

	unsigned int getNormalSize() const {
		return (unsigned int)this->Normals.size() * sizeof(float);
	}

	unsigned int getTexCoordSize() const {
		return (unsigned int)this->TexCoords.size() * sizeof(float);
	}

	unsigned int getIndexSize() const {
		return (unsigned int)this->Indices.size() * sizeof(unsigned int);
	}

	const float* getVertices() const {
		return this->Vertices.data();
	}

	const float* getPosition() const {
		return this->Position.data();
	}

	const float* getNormals() const {
		return this->Normals.data();
	}

	const float* getTexCoords() const {
		return this->TexCoords.data();
	}

	const unsigned int* getIndices() const {
		return this->Indices.data();
	}

	void showInfo() const {
		std::cout << "===== Cylinder =====\n"
			<< "   Base Radius: " << this->BaseRadius << std::endl
			<< "    Top Radius: " << this->TopRadius << std::endl
			<< "        Height: " << this->Height << std::endl
			<< "  Sector Count: " << this->Longitude << std::endl
			<< "   Stack Count: " << this->Latitude << std::endl
			<< "Triangle Count: " << getTriangleCount() << std::endl
			<< "   Index Count: " << getIndexCount() << std::endl
			<< "  Vertex Count: " << getVertexCount() << std::endl
			<< "  Position Count: " << getPositionCount() << std::endl
			<< "  Normal Count: " << getNormalCount() << std::endl
			<< "TexCoord Count: " << getTexCoordCount() << std::endl;
	}
	
private:
	float BaseRadius;
	float TopRadius;
	float Height;
	unsigned int Longitude;
	unsigned int Latitude;
	unsigned int BaseIndex;
	unsigned int TopIndex;
	std::vector<float> UnitCircleVertices;
	std::vector<float> Vertices;
	std::vector<float> Position;
	std::vector<float> Normals;
	std::vector<float> TexCoords;
	std::vector<unsigned int> Indices;

	void clearVectors() {
		std::vector<float>().swap(this->Vertices);
		std::vector<float>().swap(this->Position);
		std::vector<float>().swap(this->Normals);
		std::vector<float>().swap(this->TexCoords);
		std::vector<unsigned int>().swap(this->Indices);
	}
	
	void generateVertices() {

		// Clear memory of prev arrays
		this->clearVectors();

		float x, y, z;
		float radius;

		std::vector<float> sideNormals = this->generateSideNormals();

		for (unsigned int i = 0; i <= this->Latitude; i++) {
			z = -(this->Height * 0.5f) + (float)i / this->Latitude * this->Height;
			radius = this->BaseRadius + (float)i / this->Latitude * (this->TopRadius - this->BaseRadius);
			float t = 1.0f - (float)i / this->Latitude;

			for (unsigned int j = 0, k = 0; j <= this->Longitude; j++, k += 3) {
				x = this->UnitCircleVertices[k];
				y = this->UnitCircleVertices[k + 1];
				this->addPosition(x * radius, y * radius, z);
				this->addNormal(sideNormals[k], sideNormals[k + 1], sideNormals[k + 2]);
				this->addTexCoord((float)j / this->Longitude, t);
			}
		}

		unsigned int baseVertexIndex = (unsigned int)this->Vertices.size() / 8;
		z = -this->Height * 0.5f;
		this->addPosition(0, 0, z);
		this->addNormal(0, 0, -1);
		this->addTexCoord(0.5f, 0.5f);
		for (unsigned int i = 0, j = 0; i < this->Longitude; i++, j += 3) {
			x = this->UnitCircleVertices[j];
			y = this->UnitCircleVertices[j + 1];
			this->addPosition(x * this->BaseRadius, y * this->BaseRadius, z);
			this->addNormal(0, 0, -1);
			this->addTexCoord(-x * 0.5f + 0.5f, -y * 0.5f + 0.5f);
		}

		unsigned int topVertexIndex = (unsigned int)this->Vertices.size() / 8;
		z = this->Height * 0.5f;
		this->addPosition(0, 0, z);
		this->addNormal(0, 0, 1);
		this->addTexCoord(0.5f, 0.5f);
		for (unsigned int i = 0, j = 0; i < this->Longitude; i++, j += 3) {
			x = this->UnitCircleVertices[j];
			y = this->UnitCircleVertices[j + 1];
			this->addPosition(x * this->TopRadius, y * this->TopRadius, z);
			this->addNormal(0, 0, 1);
			this->addTexCoord(x * 0.5f + 0.5f, -y * 0.5f + 0.5f);
		}

		unsigned int k1, k2;
		for (unsigned int i = 0; i < this->Latitude; i++) {
			k1 = i * (this->Longitude + 1);
			k2 = k1 + this->Longitude + 1;
			for (unsigned int j = 0; j < this->Longitude; j++, k1++, k2++) {
				this->addIndices(k1, k2, k1 + 1);
				this->addIndices(k2, k2 + 1, k1 + 1);
			}
		}

		this->BaseIndex = (unsigned int)this->Indices.size();
		for (unsigned int i = 0, k = baseVertexIndex + 1; i < this->Longitude; i++, k++) {
			if (i < (this->Longitude - 1)) {
				this->addIndices(baseVertexIndex, k + 1, k);
			} else {
				this->addIndices(baseVertexIndex, baseVertexIndex + 1, k);
			}
		}

		this->TopIndex = (unsigned int)this->Indices.size();
		for (unsigned int i = 0, k = topVertexIndex + 1; i < this->Longitude; i++, k++) {
			if (i < (this->Longitude - 1)) {
				this->addIndices(topVertexIndex, k, k + 1);
			}
			else {
				this->addIndices(topVertexIndex, k, topVertexIndex + 1);
			}
		}
	}
	
	void generateUnitCircleVertices() {
		float sectorStep = 2 * M_PI / this->Longitude;
		float sectorAngle;

		for (unsigned int i = 0; i <= this->Longitude; i++) {
			sectorAngle = i * sectorStep;
			this->UnitCircleVertices.push_back(cos(sectorAngle));
			this->UnitCircleVertices.push_back(sin(sectorAngle));
			this->UnitCircleVertices.push_back(0);
		}
	}

	void addPosition(float x, float y, float z) {
		this->Position.push_back(x);
		this->Position.push_back(y);
		this->Position.push_back(z);
		
		this->Vertices.push_back(x);
		this->Vertices.push_back(y);
		this->Vertices.push_back(z);
		// std::cout << "x:" << x << ", y:" << y << ", z:" << z << std::endl;
	}

	void addNormal(float nx, float ny, float nz) {
		this->Normals.push_back(nx);
		this->Normals.push_back(ny);
		this->Normals.push_back(nz);
		
		this->Vertices.push_back(nx);
		this->Vertices.push_back(ny);
		this->Vertices.push_back(nz);
	}

	void addTexCoord(float u, float v) {
		this->TexCoords.push_back(u);
		this->TexCoords.push_back(v);
		
		this->Vertices.push_back(u);
		this->Vertices.push_back(v);
	}

	void addIndices(unsigned int i1, unsigned int i2, unsigned int i3) {
		this->Indices.push_back(i1);
		this->Indices.push_back(i2);
		this->Indices.push_back(i3);
		// std::cout << "i1:" << i1 << ", i2:" << i2 << ", i3:" << i3 << std::endl;
	}
	
	// Generate shared normal vectors of the side of cylinder
	std::vector<float> generateSideNormals() {
		std::vector<float> sideNormal;
		float sectorStep = M_PI * 2 / this->Longitude;
		float sectorAngle;

		// Compute the normal vector at 0 degree first
		float zAngle = atan2(this->BaseRadius - this->TopRadius, this->Height);
		float x0 = cos(zAngle);
		float y0 = 0;
		float z0 = sin(zAngle);

		std::vector<float> normals;
		for (unsigned int i = 0; i <= this->Longitude; i++) {
			sectorAngle = i * sectorStep;
			sideNormal.push_back(cos(sectorAngle) * x0 - sin(sectorAngle) * y0);
			sideNormal.push_back(sin(sectorAngle) * x0 + cos(sectorAngle) * y0);
			sideNormal.push_back(z0);
		}

		return sideNormal;
	}
};