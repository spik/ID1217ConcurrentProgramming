package controller;

import java.rmi.RemoteException;
import java.util.ArrayList;

import elevator.rmi.Elevator;

/**
 * This class is a handler for the elevators
 * @author Charlotta Spik and Nahida Islam
 */
public class ElevatorHandler extends Thread{

	Elevator elevator;
	int elevatorId;
	double currentPosition = 0;
	int destination;
	int direction;
	Task currentTask;
	int destinationFloor;

	ArrayList<Task> taskList = new ArrayList<Task>();

	static final int DIRECTION_UP = 1;
	static final int DIRECTION_DOWN = -1;
	static final int DIRECTION_NONE = 0;

	public ElevatorHandler(Elevator elevator, int elevatorId) {
		this.elevator = elevator;
		this.elevatorId = elevatorId;
	}

	/**
	 * This is the method that is run when the processes starts. It waits for the task and performs it
	 */
	public void run(){
		super.run();
		try{
			while(true){
				
				//If the tasklist is empty...
				if(taskList.isEmpty()){
					
					//wait for a task to come in
					synchronized(this){
						wait();
					}
					//When a task has been added to the list, get the task from the list 
					currentTask = taskList.get(0);
					
					//Perform the task
					performTask(currentTask);
				}
				//In this case, the list is not empty but the current task is null
				else if(currentTask == null){
					
					//Set current task to the next task in the list
					currentTask = taskList.get(0);
					
					//perform task
					performTask(currentTask);
				}

				currentPosition = getCurrentPosition();

				//Get the nice floor graphics 
				elevator.setScalePosition((int)(currentPosition + 0.05));

				//If the current position is between the destination floor -0,05 and destination floor + 0.05,
				//the elevator is at the destination floor. 
				//This is because a step of the elevator is 0.05, so we have to check if the current position is within 
				//a 0,05 interval of the destination floor
				if (currentPosition >= destinationFloor - 0.05 && currentPosition <= destinationFloor + 0.05) {
					
					//If the elevator is at the bottom or top floor we reset the direction to 0
					if (currentPosition >=0 && currentPosition <= 0.05 || currentPosition >=4.95 && currentPosition <= 5) {
						direction = 0;
					}
					stopElevator();
					elevator.open();
					taskList.remove(currentTask);
					currentTask = null;
				}
			}
		}
		catch(Exception e){
			e.printStackTrace();
		}
	}
	
	/**
	 * This method decides what direction the elevator should move in and start the elevator moving in that direction
	 * @param task The current task that determines the direction
	 */
	public void decideDirection(Task task){
		currentPosition = getCurrentPosition();

		try{
			//the elevator is above the destination so the elevator needs to go down
			if(currentPosition > (double)task.destinationFloor + 0.05){
				elevator.down();
				direction = DIRECTION_DOWN;
				System.out.println("elevator " + elevatorId + " going down");
			}
			//the elevator is below the destination so the elevator needs to go up
			else if(currentPosition < (double)task.destinationFloor - 0.05){
				elevator.up();
				direction = DIRECTION_UP;
				System.out.println("elevator " + elevatorId + " going up");
			}
			//the elevator is at the destination so the elevator should not move
			else{
				direction = DIRECTION_NONE;
			}
		}
		catch (RemoteException e) {
			e.printStackTrace();
		}
	}

	/**
	 * This method sets the destination floor and closes the doors of the elevators
	 * @param task The task that is performed
	 */
	public void performTask(Task task){
		destinationFloor = task.destinationFloor;
		try {
			Thread.sleep(1000);
			elevator.close();
			Thread.sleep(1000);	
		} catch (Exception e) {
			e.printStackTrace();
		}
		decideDirection(task);
	}
	/**
	 * This method calls elevator.stop() to stop the elevator
	 */
	public void stopElevator(){
		try {
			elevator.stop();
		} catch (RemoteException e) {
			e.printStackTrace();
		}
	}

	//This method calls elevetor.whereIs to get the current position of the elevator 
	public double getCurrentPosition(){
		try {
			currentPosition = elevator.whereIs();
		} catch (RemoteException e) {
			e.printStackTrace();
		}
		return currentPosition;
	}

	/**
	 * This method adds a task to the tasklist
	 * @param task The task to be added to the tasklist
	 */
	public void addTask(Task task) {

		boolean isDuplicateTask = false;

		//Loop througH all tasks in the list
		for (Task current : taskList) {

			//Check if duplicate. If it is, don't add the task in the tasklist
			if(current.destinationFloor == task.destinationFloor && current.direction == task.direction){
				System.out.println("Task is already in the list");
				isDuplicateTask = true;
				break;
			}
		}
		//If it is not a duplicate
		if (isDuplicateTask == false) {
			
			//Loop through all tasks in the list in order to place the new task in the right position, i.e. sort list
			for (int i = 0; i < taskList.size(); i++){
				
				//task.direction == direction: Have to check if the right direction so that when pressing for example 5, 3 up, 3 down it doesn't just stop once on floor 3
				//direction * task.destinationFloor <= taskList.get(i).destinationFloor * direction: Check if the destination floor for the task we want to add is less than the destination floor of the task i
				//If it is, we want to place the task in position i in the queue instead of at the back. 
				//Because of this we will perform the task with the lowest distance to destination floor first and so on
				//direction * currentPosition <= direction * task.destinationFloor: AND we have NOT passed the floor yet (for if we have, we need to add the task in the back of the list).
				//task.direction == 0 -> the task direction is not set, the command is from inside the elevator
				if ((task.direction == getDirection(task) || task.direction == 0) && getDirection(task) * task.destinationFloor < taskList.get(i).destinationFloor * getDirection(task) 
						&& getDirection(task) * currentPosition < getDirection(task) * task.destinationFloor) {
					System.out.println("test1");
					placeTask(task, i);
					return;
				}
			}
			//In this case, all tasks in the tasklist had a destination that was nearer than the task we want to add.
			//Therefore, add the task last in the list. 
			taskList.add(task);
			System.out.println("A task was added to elevator " + elevatorId + " with destination " + task.destinationFloor);

			//Notify the elevator that it has gotten a task
			synchronized(this){
				notify();
			}
			currentTask = taskList.get(0);
			destinationFloor = currentTask.destinationFloor;
		}
	}

	/**
	 * Places the task in the right position of the list and notifies the elevator that a task is added
	 * @param task The task which is to be added to the list
	 * @param i The position of the list where the task should be added 
	 */
	private void placeTask(Task task, int i) {
		
		//Add task in position i
		taskList.add(i, task);
		System.out.println("A task was added to elevator " + elevatorId + " with destination " + task.destinationFloor);

		//Notify the elevator that it has gotten a task
		synchronized(this){
			notify();
		}
		currentTask = taskList.get(0);
		destinationFloor = currentTask.destinationFloor;
	}
	
	public int getDirection(Task task){
		
		int direction;
		//the elevator is above the destination so the elevator needs to go down
		if(currentPosition > (double)task.destinationFloor + 0.05){
			direction = -1;
		}
		//the elevator is below the destination so the elevator needs to go up
		else if(currentPosition < (double)task.destinationFloor - 0.05){
			direction = 1;
		}
		//the elevator is at the destination so the elevator should not move
		else{
			direction = 0;
		}
		return direction;
	}
}

