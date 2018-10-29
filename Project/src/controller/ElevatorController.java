package controller;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.rmi.RemoteException;

import elevator.rmi.Elevator;
import elevator.rmi.MakeAll;

/**
 * This is a controller for the elevators
 * @author Charlotta Spik and Nahida Islam
 */
public class ElevatorController extends Thread implements ActionListener {

	Elevator[] elevators;
	ElevatorHandler[] handlers;
	String rmihost;
	int intendedDirection;
	int intendedFloor;
	int elevatorId;

	/**
	 * Constructor for ElevatorController. Sets the RMI host to localhost
	 */
	public ElevatorController(){
		rmihost = "localhost";
	}

	/**
	 * This method is run when the thread is started. Creates and starts all the elevator threads
	 */
	public void run(){
		try{
			MakeAll.init(rmihost);
			MakeAll.addFloorListener(this);
			MakeAll.addInsideListener(this);

			//Initialize an array with the size of the number of elevators 
			elevators = new Elevator[MakeAll.getNumberOfElevators()];
			handlers = new ElevatorHandler[MakeAll.getNumberOfElevators()];

			//Fill the arrays with elevators/elevatorHandlers and start ElevatorHandler processes
			for (int i = 0; i < elevators.length ; i++) {
				elevators[i] = MakeAll.getElevator(i + 1);
				handlers[i] = new ElevatorHandler(elevators[i], i + 1);
				handlers[i].start();		
			}
		}
		catch(Exception ex){
			ex.printStackTrace();
		}	
	}

	/**
	 * main method. Start the ElevatorController process
	 * @param args
	 */
	public static void main(String[] args){
		new ElevatorController().start();
	}

	/**
	 * This method parses the action event received from a button press to determine what action the elevator should take
	 * @param e The action event that is parsed
	 */
	public void actionPerformed(ActionEvent e){
		try{
			//get the action from the event
			String action = e.getActionCommand();

			//Split the given argument and place in an array 
			String[] arg= action.split(" ");

			//Make a switch case based on the first argument in the array
			switch(arg[0]){

			//If the first argument is p, the action is from inside the elevator
			case "p":

				//The ID of the elevator is the second argument minus one, as elevator 1 is on position 0 in the array
				elevatorId = Integer.parseInt(arg[1]) - 1; 

				//32000 is the action for the emergency stop button
				if(action.contains("32000")){

					//If the emergency stop button is pressed, stop the elevator. 
					handlers[elevatorId].stopElevator();
				}
				//In this case another button inside the elevator was pressed
				else{
					//get the floor the person inside the elevator wants to go to from the third argument
					int floor = Integer.parseInt(arg[2]);

					//Create a task for this floor
					Task task = new Task(floor);

					//Add the task in the tasklist 
					handlers[elevatorId].addTask(task);
				}
				break;

			//If the first argument is b, the action is from a floor
			case "b":

				//Get the floor from which the button is pressed
				intendedFloor = Integer.parseInt(arg[1]);

				//Get the direction from the second argument
				intendedDirection = Integer.parseInt(arg[2]);

				//Create a new task for the floor
				Task task = new Task(intendedFloor);

				//Set the direction 
				task.setDirection(intendedDirection);

				//Assign the task to an elevator. Which elevator gets it depends on the cost. 
				assigner(task);

				break;
			//this is the default case for if there are no case matches. This should never happen. 
			default:
				System.out.println("Error, invalid command");
				break;
			}
		}
		catch(Exception ex){
			ex.printStackTrace();
		}	
	}

	/**
	 * Assign a task to the elevator with lowest cost
	 * @param task The task to be assigned to an elevator to perform 
	 * @throws RemoteException 
	 */
	public void assigner(Task task) throws RemoteException {

		//Initialize the min value and cost to the maximum value of a double for now
		double min, cost;
		min = cost = Double.MAX_VALUE;

		//Initialize the ID of the nearest elevator to the first elevator
		int nearestElevatorId = 0;

		//Go through all elevators and find the elevator with the lowest cost
		for (int i = 0; i < handlers.length; i++) {
			
			//Get the current position of the elevator
			double currentPosition = handlers[i].getCurrentPosition();
			
			//Get the direction of the elevator
			int direction = handlers[i].direction;

			System.out.println(" - Elevator " + (i + 1) + "'s direction is: " + direction);
			System.out.println(" - Intended direction is: " + intendedDirection);

			//If the elevators direction is the same as the direction the person on the floor wants to go AND
			//The elevator has not yet passed the floor
			//multiply by the direction to get negative when going down and positive when going up
			//This allows us to have just one case that is true for whatever direction the elevator is going in
			if (intendedDirection == direction && direction * currentPosition < direction * intendedFloor) {

				//Calculate the distance between where the elevator is and where the destination floor
				cost = Math.abs(intendedFloor - currentPosition);
				if (cost < min) {
					min = cost;
					nearestElevatorId = i;
				}
			}
			//This elevator has nothing to do
			else if(direction == 0){
				cost = Math.abs(intendedFloor - currentPosition);
				if (cost < min) {
					min = cost;
					nearestElevatorId = i;
				}
			}
			else{
				System.out.println("Elevator " + handlers[i].elevatorId + " is going in the wrong direction or has passed the floor");
			}
		}

		System.out.println("Elevator " + (nearestElevatorId + 1) + " has the lowest cost");
		
		//Set the direction of the elevator to the direction it is going to move soon
		handlers[nearestElevatorId].direction = handlers[nearestElevatorId].getDirection(task);
		handlers[nearestElevatorId].addTask(task);
	}
}
