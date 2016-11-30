import java.io.*;

public class Main {
	public static int Load() throws IOException{
		String save = "0";
		String line = "0";
		BufferedReader br = new BufferedReader(new FileReader("d:/out.txt"));
		while(true) {
			save = line;
			line = br.readLine();
            if (line==null) break;
        }
		br.close();
		return Integer.parseInt(save);
		
	}
	
	public static void Store(int data) throws IOException{
		 BufferedWriter file = new BufferedWriter(new FileWriter("d:/out.txt", true));
		 file.write(Integer.toString(data), 0, Integer.toString(data).length());
		 file.newLine();
		 file.close();
	}
	public static int Add(int i,int j) {
		   return i+j;
	}
	
	public static void main(String[] args) throws IOException{
		for (int i=0; i<1000; i++) 
		{			
			   int x= Load();			   
			   x=Add(x,1);			   
			   Store(x);
		}

	}

}


