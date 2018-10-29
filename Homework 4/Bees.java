/**
 * Given are n honeybees and a hungry bear. They share a pot of honey. The pot is initially empty; 
 * its capacity is H portions of honey. The bear sleeps until the pot is full, then eats all the honey 
 * and goes back to sleep. Each bee repeatedly gathers one portion of honey and puts it in the pot; 
 * the bee who fills the pot awakens the bear.
 * 
 * Fairness: The solution is not fair as synchronized blocks/methods hold no fairness guarantee so we 
 * cannot know which thread will run after the current thread exits the synchronized block. Fairness 
 * can be implemented by using special locks with fairness options available in Java, or by using a 
 * queue for the threads so that a waiting thread gets put in a queue while waiting to enter the 
 * synchronized block. 
 */
public class Bees {

	static int H = 7;
	static int honey = 0;
	static int bees = 7;

	public static void main(String[] args) {
		Bowl bowl = new Bowl();
		Bear bear = new Bear(bowl);
		
		//Start the bear process
		bear.start();
		
		//Start all bee processes 
		for (int i = 0; i < bees; i++) {
			Bee bee = new Bee(i, bowl);
			bee.start();
		}
	}

	/**
	 * Makes a thread sleep for the specified time
	 * @param ms Time in milliseconds to sleep
	 */
	public static void msSleep(int ms){
		try {
			Thread.sleep(ms);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
	
	/**
	 * This is the monitor. Keeps track of how much honey is in the bowl
	 */
	public static class Bowl{
		
		/**
		 * Get some honey from the bowl
		 */
		public synchronized void get(){
			
			//The bowl is full and the bear was signaled by the bees to wake up
			System.out.println("Bear wakes up and goes to eat some honey");
			
			//As long as there is honey in the bowl
			while(honey > 0){
				
				//Eat some honey
				honey--;
				System.out.println(" - Bear ate honey, " + honey + " units of honey left in the bowl");
				
				//Go to sleep
				msSleep(500);

			}
			//The bowl is now empty
			System.out.println("The bowl is empty, notify bees to make more and go to sleep");
			
			//Wake all the bees to make more
			notifyAll();
			try {
				//Wait for bees to fill the bowl
				wait();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		/**
		 * Put honey in the bowl
		 * @param id ID of the bee that puts honey in the bowl
		 */
		public synchronized void put(int id){

			//The bowl is not full
			if(honey < H-1){
				
				//Put honey in the bowl
				honey++;
				System.out.println(" - Bee " + id + " put honey in the bowl, " + honey + " units of honey in bowl");
				
				//Sleep for a while after working so hard
				msSleep(500);
			}
			//The bowl is almost full
			else if(honey == H-1){
				
				//Fill the bowl by adding the last unit of honey
				honey++;
				System.out.println(" - Bee " + id + " fills bowl, signal bear to wake up");
				
				//Signal bear that the bowl is now full
				notify();
				
				//Sleep for a while after working so hard
				msSleep(500);

				try {
					//Wait for bowl to be empty
					wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}

			}
			//The bowl is full
			else{
				try {
					//Wait for bowl to be empty
					wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}

	}
	/**
	 * This class represents the bear
	 */
	public static class Bear extends Thread{
		private Bowl bowl;
		
		/**
		 * Constructor
		 * @param b This is the instance of the class "Bowl"
		 */
		public Bear(Bowl b){
			bowl = b;
		}
		/**
		 * Method to be run by the thread
		 */
		public void run(){
			while(true){
				
				//Get some honey from the bowl
				bowl.get();
				
				//Sleep
				msSleep(500);
			}
		}
	}
	/**
	 * This class represents the bee
	 */
	public static class Bee extends Thread{
		private Bowl bowl;
		private int beeID;
		/**
		 * Constructor
		 * @param id The ID of the bee
		 * @param b This is the instance of the class "Bowl"
		 */
		public Bee(int id, Bowl b){
			bowl = b;
			beeID = id;
		}
		/**
		 * Method to be run by the threads
		 */
		public void run(){
			while(true){
				
				//Put honey in bowl
				bowl.put(beeID);
				
				//Sleep
				msSleep(300);
			}
		}
	}

}
