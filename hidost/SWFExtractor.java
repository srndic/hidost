import tv.porst.swfretools.dissector.console.ConsoleDumper;

public class SWFExtractor {

	/**
	 * @param args - command-line arguments; this program 
	 *               requires the path to the SWF file to 
	 *               be analyzed as its first argument
	 */
	public static void main(String[] args) {
		ConsoleDumper.dump(args[0]);
		System.out.println("OKAY - DONE!");
	}

}
