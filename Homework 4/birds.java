/**
 * Given are n baby birds and one parent bird. The baby birds eat out of a common dish that initially containsW worms. 
 * Each baby bird repeatedly takes a worm, eats it, sleeps for a while, takes another worm, and so on. If the dish is 
 * empty, the baby bird who discovers the empty dish chirps real loud to awaken the parent bird. The parent bird flies 
 * off and gathers W more worms, puts them in the dish, and then waits for the dish to be empty again. This pattern 
 * repeats forever.
 * 
 * Fairness: The solution is not fair as synchronized blocks/methods hold no fairness guarantee so we 
 * cannot know which thread will run after the current thread exits the synchronized block. Fairness 
 * can be implemented by using special locks with fairness options available in Java, or by using a 
 * queue for the threads so that a waiting thread gets put in a queue while waiting to enter the 
 * synchronized block. 
 */
public class birds {

	static int W = 5;
	static int worms = 5;
	static int birds = 5;

	public static void main(String[] args) throws InterruptedException {

		Dish dish = new Dish();
		Parent_bird parent = new Parent_bird(dish);
		
		//Start the parent bird process 
		parent.start();
		
		//Start the baby bird processes
		for (int i = 0; i < birds; i++) {
			Baby_bird bird = new Baby_bird(i, dish);
			bird.start();
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
	 * This is the monitor
	 */
	public static class Dish{
		
		/**
		 * Get a worm from the dish
		 * @param birdID The ID of the bird that gets the worm
		 */
		public synchronized void get(int birdID){

			//If there are worms in the dish
			if(worms > 1){
				
				//eat a worm
				worms--;
				System.out.println(" - Bird " + birdID + " ate worm and fell asleep, " + worms + " worms left in the dish");
				
				//Sleep after eating
				msSleep(500);
			}
			//The bird eating the last worm calls the parent
			else if(worms == 1){
				
				//eat the last worm
				worms--;
				System.out.println(" - Bird " + birdID + " ate the last worm, chirps to signal parent");
				
				//Signal parent that the dish is empty
				notify();
				
				//Sleep after eating
				msSleep(500);
				try {
					//Wait for parent to fill the bowl
					wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			//There are no worms in the dish
			else{
				try {
					//Wait for parent to put worms in the dish
					wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}	
		}
		/**
		 * Put a worm in the dish
		 */
		public synchronized void put(){

			//The dish is not empty
			while(worms != 0){

				//Wait for birds to eat all worms
				try {
					wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}

			//The dish is empty
			System.out.println("Parent hears baby's chirps, leaves to find more worms");

			//Looking for worms
			msSleep(300);

			//Get W worms
			worms = W;
			System.out.println("Parent found " + worms + " worms");

			//Signal all birds that the dish is full
			notifyAll();
		}
	}
	/**
	 * This class represents the parent bird
	 */
	public static class Parent_bird extends Thread{

		public Dish dish;

		public Parent_bird(Dish d){
			dish = d;
		}

		public void run(){
			while(true){
				
				//Put worms in the dish
				dish.put();
				
				//Sleep for a while after working so hard to find food
				msSleep(300);
			}
		}
	}
	/**
	 * This class represent the baby birds
	 */
	public static class Baby_bird extends Thread{

		public Dish dish;
		int birdID;
		public Baby_bird(int id, Dish d){
			birdID = id;
			dish = d;
		}
		public void run(){
			while(true){
				
				//Get worms from the dish
				dish.get(birdID);

				//Sleep for a while to give the other birds a chance to eat
				msSleep(300);
			}
		}
	}
}
