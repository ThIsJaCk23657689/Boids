#pragma once
#define _USE_MATH_DEFINES

#include <vector>

const unsigned int MIN_LONGITUDE = 3;
const unsigned int MIN_LATITUDE = 1;

class Cylinder
{
public:

	Cylinder(float baseRadius = 1.0f, float topRadius = 1.0f, float height = 1.0f, unsigned int longitude = 30, unsigned int latitude = 30, bool smooth = true) {
		this->set(baseRadius, topRadius, height, longitude, latitude, smooth);
	}
	~Cylinder();

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
	
	void set(float baseRadius, float topRadius, float height, unsigned int longitude, unsigned int latitude, bool smooth) {
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

		this->Smooth = smooth;

		generateUnitCircleVertices();

		if (this->Smooth) {
			generateVerticesSmooth();
		} else {
			generateVerticesFlat();
		}
	}

	void setBaseRadius(float radius) {
		if (this->BaseRadius != radius) {
			this->set(radius, this->TopRadius, this->Height, this->Longitude, this->Latitude, this->Smooth);
		}
	}
	
	void setTopRadius(float radius) {
		if (this->TopRadius != radius) {
			this->set(this->BaseRadius, radius, this->Height, this->Longitude, this->Latitude, this->Smooth);
		}
	}
	
	void setHeight(float height) {
		if (this->Height != height) {
			this->set(this->BaseRadius, this->TopRadius, height, this->Longitude, this->Latitude, this->Smooth);
		}
	}
	
	void setLongitude(unsigned int longitude) {
		if (this->Longitude != longitude) {
			this->set(this->BaseRadius, this->TopRadius, this->Height, longitude, this->Latitude, this->Smooth);
		}
	}
	
	void setLatitude(unsigned int latitude) {
		if (this->Latitude != latitude) {
			this->set(this->BaseRadius, this->TopRadius, this->Height, this->Longitude, latitude, this->Smooth);
		}
	}
	
	void setSmooth(bool smooth) {
		if (this->Smooth == smooth) {
			return;
		}

		this->Smooth = smooth;
		if (smooth) {
			generateVerticesSmooth();
		} else {
			generateVerticesFlat();
		}
	}

	unsigned int getVertexCount() const {
		return (unsigned int)this->Vertices.size() / 3;
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

	unsigned int getLineIndexCount() const {
		return (unsigned int)this->LineIndices.size();
	}

	unsigned int getTriangleCount() const {
		return (unsigned int)this->getIndexCount() / 3;
	}

	unsigned int getVertexSize() const {
		return (unsigned int)this->Vertices.size() * sizeof(float);
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

	unsigned int getLineIndexSize() const {
		return (unsigned int)this->LineIndices.size() * sizeof(unsigned int);
	}

	const float* getVertices() const {
		return this->Vertices.data();
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

	const unsigned int* getLineIndices() const {
		return this->LineIndices.data();
	}

	void draw() const {
		
	}
	
private:
	float BaseRadius;
	float TopRadius;
	float Height;
	unsigned int Longitude;
	unsigned int Latitude;
	bool Smooth;
	std::vector<float> UnitCircleVertices;
	std::vector<float> Vertices;
	std::vector<float> Normals;
	std::vector<float> TexCoords;
	std::vector<unsigned int> Indices;
	std::vector<unsigned int> LineIndices;

	void clearVectors() {
		std::vector<float>().swap(this->Vertices);
		std::vector<float>().swap(this->Normals);
		std::vector<float>().swap(this->TexCoords);
		std::vector<unsigned int>().swap(this->Indices);
		std::vector<unsigned int>().swap(this->LineIndices);
	}
	
	void generateVerticesSmooth() {

		// Clear memory of prev arrays
		this->clearVectors();

		float x, y, z;
		float s, t;
		float radius;

		std::vector<float> unitVertices = generateUnitCircleVertices();

		for (unsigned int i = 0; i < 2; i++) {
			float h = -this->Height / 2.0f + i * this->Height;
			float t = 1.0f - i;

			for (unsigned int j = 0, k = 0; j <= this->Longitude; j++, k += 3) {
				float ux = unitVertices[k];
				float uy = unitVertices[k + 1];
				float uz = unitVertices[k + 2];

				this->Vertices.push_back(ux);
				this->Vertices.push_back(uy);
				this->Vertices.push_back(h);

				this->Normals.push_back(ux);
				this->Normals.push_back(uy);
				this->Normals.push_back(uz);

				this->TexCoords.push_back((float)j / this->Longitude);
				this->TexCoords.push_back(t);
			}
		}
	}

	void generateVerticesFlat() {
		
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

	// Generate shared normal vectors of the side of cylinder
	std::vector<float> generateSideNormals() {
		float sectorStep = 2 * M_PI / this->Longitude;
		float sectorAngle;

		// Compute the normal vector at 0 degree first
		float zAngle = atan2(this->BaseRadius - this->TopRadius, Height);
		float x0 = cos(zAngle);
		float y0 = 0;
		float z0 = sin(zAngle);

		std::vector<float> normals;
		for (unsigned int i = 0; i <= this->Longitude; i++) {
			
		}

		return this->Normals;
	}
};