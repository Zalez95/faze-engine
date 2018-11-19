#ifndef CONSTRAINT_MANAGER_H
#define CONSTRAINT_MANAGER_H

#include <array>
#include <vector>
#include <glm/glm.hpp>

namespace se::physics {

	class Constraint;
	struct RigidBody;


	/**
	 * Class ConstraintManager, it's the class used to solve the physic
	 * constraints between the rigid bodies of the Physics System.
	 */
	class ConstraintManager
	{
	private:	// Nested types
		using vec12 = std::array<float, 12>;

	private:	// Attributes
		/** The maximum number of iterations that the Gauss-Seidel algorithm
		 * should run */
		static constexpr int kMaxIterations = 16;

		/** The vector that holds the registered Constraints in the
		 * ConstraintManager */
		std::vector<Constraint*> mConstraints;

		/** The vector that holds the RigidBodies whose movement is being will
		 * by the Constraints */
		std::vector<RigidBody*> mRigidBodies;

		/** The vector that stores the relations between the RigidBodies and
		 * its Constraints */
		std::vector<std::array<int, 2>> mConstraintRBMap;

		/** The variable that will be solved by the Constraint resolver.
		 * It's a column matrix with the size of (number of Constraints). */
		std::vector<float> mLambdaMatrix;

		/** A matrix with the lower bounds of the lambda variable. It's a
		 * column matrix with the size of (number of Constraints). */
		std::vector<float> mLambdaMinMatrix;

		/** A matrix with the upper bounds of the lambda variable. It's a
		 * column matrix with the size of (number of Constraints). */
		std::vector<float> mLambdaMaxMatrix;

		/** A matrix with all the Constraints' biases. It's a row matrix with
		 * the size of (number of Constraints). */
		std::vector<float> mBiasMatrix;

		/** The diagonal of the inverse matrix with all the masses
		 * (E3 * mass and the inertia tensor) of all the RigidBodies.
		 * It's a square matrix with a size of 2*(number of RigidBodies)
		 * mat3s. */
		std::vector<glm::mat3> mInverseMassMatrix;

		/** A matrix with the velocities (linear and angular) of all the
		 * RigidBodies. It's a column matrix with a size of
		 * 2*(number of RigidBodies) vec3s. */
		std::vector<glm::vec3> mVelocityMatrix;

		/** A matrix with the external forces (forces and torques) of all the
		 * RigidBodies. It's a column matrix with a size of
		 * 2*(number of RigidBodies) vec3s. */
		std::vector<glm::vec3> mForceExtMatrix;

		/** A matrix with the jacobians of all the Constraints. It's a matrix
		 * with a size of (number of Constraints) by 6*(number of RigidBodies),
		 * but in our case it's represented by 12*(number of Constraints)
		 * floats so we can access to its data with the ConstraintRBMap. */
		std::vector<vec12> mJacobianMatrix;

	public:		// Constraints
		/** Creates a new ConstraintManager */
		ConstraintManager() {};

		/** Class destructor */
		~ConstraintManager() {};

		/** Registers the given Constraint in the ConstraintManager, so the
		 * movement of the RigidBodies that it holds will be restricted.
		 *
		 * @param	constraint a pointer to the Constraint that we want to
		 *			register */
		void addConstraint(Constraint* constraint);

		/** Removes the given Constraint from the ConstraintManager, so the
		 * movement of the RigidBodies that it holds won't longer be
		 * restricted.
		 *
		 * @param	constraint a pointer to the Constraint that we want to
		 *			remove */
		void removeConstraint(Constraint* constraint);

		/** Applies the constraints stored in the ConstraintManager
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		void update(float deltaTime);
	private:
		/** Updates the bias matrix value */
		void updateBiasMatrix();

		/** Updates the velocity matrix value */
		void updateVelocityMatrix();

		/** Updates the external forces matrix value */
		void updateForceExtMatrix();

		/** Updates the jacobian matrix value */
		void updateJacobianMatrix();

		/** Runs the Gauss-Seidel algorithm for solving the mLambdaMatrix in:
		 * mJacobianMatrix * mInverseMassMatrix * transpose(mJacobianMatrix)
		 *	* mLambdaMatrix = etaMatrix
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		void calculateGaussSeidel(float deltaTime);

		/** Calculates the transposed invMassJacobianMatrix which is
		 * equal to mInverseMassMatrix * transpose(mJacobianMatrix)
		 *
		 * @return	the invMassJacobianMatrix matrix. Its size is the same than
		 *			(number of RigidBodies) * (number of Constraints), but 
		 *			because it has the same sparsity than mJacobianMatrix
		 *			it is stored in the same way */
		std::vector<vec12> calculateInvMassJacobianMatrix() const;

		/** Calculates the Eta Matrix, which is equal to
		 * mBiasMatrix / deltaTime - mJacobianMatrix *
		 * (mVelocityMatrix / deltaTime + mInverseMassMatrix * mForceExtMatrix)
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds
		 * @return	the Eta matrix. Its size is (number of Constraints) */
		std::vector<float> calculateEtaMatrix(float deltaTime) const;

		/** Calculates the invMJLambdaMatrix which is equal to
		 * B * lambda
		 *
		 * @param	invMassJacobianMatrix the matrix equal to
		 *			mInverseMassMatrix * transpose(mJacobianMatrix)
		 * @param	lambdaMatrix the lambda matrix
		 * @return	the A matrix. Its size is 6*(number of RigidBodies) */
		std::vector<float> calculateInvMJLambdaMatrix(
			const std::vector<vec12>& invMassJacobianMatrix,
			const std::vector<float>& lambdaMatrix
		) const;

		/** Calculates the diagonal of the JMJMatrix, which is equal to,
		 * mJacobianMatrix * invMassJacobianMatrix
		 *
		 * @param	jacobianMatrix the jacobian matrix
		 * @param	invMassJacobianMatrix the matrix equal to
		 *			mInverseMassMatrix * transpose(mJacobianMatrix)
		 * @return	the diagonal of the JMJMatrix. Its size is
		 *			(number of Constraints) */
		std::vector<float> calculateDiagonalJInvMJMatrix(
			const std::vector<vec12>& jacobianMatrix,
			const std::vector<vec12>& invMassJacobianMatrix
		) const;

		/** Updates the velocity and position of the RigidBodies with the
		 * calculated lambda matrix
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		void updateRigidBodies(float deltaTime);
	};

}

#endif		// CONSTRAINT_MANAGER_H