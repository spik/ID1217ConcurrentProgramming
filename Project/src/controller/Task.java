package controller;

/**
 * This is the class for the tasks. 
 * @author Charlotta Spik and Nahida Islam
 */
public class Task {
	
	int destinationFloor;
	int direction = 0;

	/**
	 * Constructor for the Task class
	 * @param destinationFloor The floor the person on outside the elevator is on
	 */
	public Task(int destinationFloor) {
		this.destinationFloor = destinationFloor;
	}

	/**
	 * Sets the direction
	 * @param direction The direction the elevator has to go to reach the destination floor.
	 */
	public void setDirection(int direction) {
		this.direction = direction;
	}
	
	/**
	 * This method returns the tasks direction, i.e. the direction the person outside the elevator wants to go
	 * @return The direction the person outside the elevator wants to go
	 */
	public int getDirection(){
		return direction;
	}
	/**
	 * toString method for task. Used for printing the task in an understandable way.
	 * @return The string that describes the task
	 */
	public String tostring(){
		return "TASK: destination floor: " + destinationFloor + " direction: " + direction;
	}
}
