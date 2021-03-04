#pragma once

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <vector>

const float PERCEPTIONRADIUS = 5.0f;
const float MAXFORCE = 0.1f;
const float MAXSPEED = 4.0f;
const float SIZE = 1.0f;

class Boid
{
public:
	Boid(glm::vec3 position = glm::vec3(0.0f), glm::vec3 velocity = glm::vec3(1.0f), unsigned int id = 0) : PerceptionRadius(PERCEPTIONRADIUS), MaxForce(MAXFORCE), Size(SIZE) {
		this->Id = id;
		this->Position = position;
		this->Velocity = velocity;
		this->Acceleration = glm::vec3(0.0f);
		this->MaxSpeed = MAXSPEED;
		this->Model = glm::mat4(0.0f);
	}

	void flock(std::vector<Boid> boids, float s_atten, float a_atten, float c_atten) {
		unsigned int neighbors = 0;
		this->Acceleration *= 0;

		glm::vec3 avg_pushback_force = glm::vec3(0.0f);
		glm::vec3 avg_velocity = glm::vec3(0.0f);
		glm::vec3 avg_position = glm::vec3(0.0f);

		for (unsigned int i = 0; i < boids.size(); i++) {
			float distance = glm::distance(this->Position, boids[i].Position);
			if (boids[i].Id != this->Id && distance < this->PerceptionRadius) {
				// Separation
				glm::vec3 diff = this->Position - boids[i].Position;
				diff /= distance;
				avg_pushback_force += diff;

				// Alignment
				avg_velocity += boids[i].Velocity;
				
				// Cohesion
				avg_position += boids[i].Position;

				neighbors++;
			}
		}

		if (neighbors > 0) {
			avg_pushback_force /= neighbors;
			avg_pushback_force = this->setMag(avg_pushback_force, this->MaxSpeed);
			avg_pushback_force -= this->Velocity;
			avg_pushback_force = this->limit(avg_pushback_force, this->MaxForce);

			avg_velocity /= neighbors;
			avg_velocity = this->setMag(avg_velocity, this->MaxSpeed);
			avg_velocity -= this->Velocity;
			avg_velocity = this->limit(avg_velocity, this->MaxForce);

			avg_position /= neighbors;
			avg_position -= this->Position;
			avg_position = this->setMag(avg_position, this->MaxSpeed);
			avg_position -= this->Velocity;
			avg_position = this->limit(avg_position, this->MaxForce);
		}
		
		glm::vec3 separation = avg_pushback_force * s_atten;
		glm::vec3 alignment = avg_velocity * a_atten;
		glm::vec3 cohesion = avg_position * c_atten;
		
		this->Acceleration = separation + alignment + cohesion;
	}

	void edges(float x, float y, float z) {
		if (this->Position.x > x) {
			this->Position.x = -x;
		} else if (this->Position.x < -x) {
			this->Position.x = x;
		}

		if (this->Position.y > y) {
			this->Position.y = -y;
		} else if (this->Position.y < -y) {
			this->Position.y = y;
		}

		if (this->Position.z > z) {
			this->Position.z = -z;
		} else if (this->Position.z < -z) {
			this->Position.z = z;
		}
	}
	
	void update(float deltaTime) {
		this->Velocity = this->Velocity + this->Acceleration * deltaTime;
		this->Velocity = this->limit(this->Velocity, this->MaxSpeed);
		this->Position = this->Position + this->Velocity * deltaTime;
		this->Acceleration *= 0;
	}

	// Getter
	glm::mat4 getModel() const { return this->Model; }
	glm::vec3 getPosition() const { return this->Position; }
	glm::vec3 getVelocity() const { return this->Velocity; }
	glm::vec3 getAcceleration() const { return this->Acceleration; }
	unsigned int getId() const { return this->Id; }
	float getSize() const { return this->getSize(); }

	// Setter
	void setModel(glm::mat4 model) { this->Model = model; }
	
private:
	glm::mat4 Model;
	glm::vec3 Position;
	glm::vec3 Velocity;
	glm::vec3 Acceleration;
	unsigned int Id;
	float PerceptionRadius;
	float MaxForce;
	float MaxSpeed;
	float Size;
	
	glm::vec3 limit(glm::vec3 vector, float number) {
		glm::vec3 result = vector;
		if(glm::length(vector) > number) {
			result = glm::normalize(vector) * number;
		}
		return result;
	}

	glm::vec3 setMag(glm::vec3 vector, float number) {
		glm::vec3 result = glm::normalize(vector) * number;
		return result;
	}
};