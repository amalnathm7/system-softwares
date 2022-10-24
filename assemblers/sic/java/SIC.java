// This is an SIC ASSEMBLER that creates the object code 
// file of a given SIC program. The pass 1 and pass 2 
// are also displayed in the output screen.

import java.util.*;
import java.io.*;

class SIC {
	Map<String, String> op = new HashMap<String, String>();
	List<String> ps_op = new ArrayList<String>();
	Map<String, String> SYMTAB = new LinkedHashMap<String, String>();
	
	int LC = 0;
	int start;
	int length;
	int end;
	String name = "";
	
	void initialise() {
		op.put("LDA", "00");
		op.put("LDX", "04");
		op.put("LDL", "08");
		op.put("STA", "0C");
		op.put("STX", "10");
		op.put("STL", "14");
		op.put("ADD", "18");
		op.put("SUB", "1C");
		op.put("MUL", "20");
		op.put("DIV", "24");
		op.put("COMP", "28");
		op.put("TIX", "2C");
		op.put("JEQ", "30");
		op.put("JGT", "34");
		op.put("JLT", "38");
		op.put("J", "3C");
		op.put("AND", "40");
		op.put("OR", "44");
		op.put("JSUB", "48");
		op.put("RSUB", "4C");
		op.put("LDCH", "50");
		op.put("STCH", "54");
		op.put("RD", "D8");
		op.put("WD", "DC");
		op.put("TD", "E0");
		op.put("STSW", "E8");
		
		ps_op.add("WORD");
		ps_op.add("BYTE");
		ps_op.add("RESW");
		ps_op.add("RESB");
	}
	
	void pass1(String filename) throws IOException {
		File file = new File(filename);
		FileWriter intFile = new FileWriter("intm_" + filename);
		Scanner scan = new Scanner(file);
		int lineNum = 1;
		
		displayPgm(filename);
		
		while(scan.hasNextLine()) {
			try {
				String str = scan.nextLine();
				String[] arr = str.split("\t");
				
				if(!arr[0].isEmpty()) {
					if(!SYMTAB.containsKey(arr[0]))
						SYMTAB.put(arr[0], Integer.toHexString(LC).toUpperCase());
					else {
						System.out.println("\n" + lineNum + ": Label \"" + arr[0] + "\" already used. Aborted.");
						intFile.close();
						scan.close();
						(new File("intm_" + filename)).delete();
						System.exit(0);
					}
				}
				
				if(arr[1].equals("START")) {
					if(arr.length == 3) {
						LC = Integer.parseInt(arr[2], 16);
						start = LC;
					}
					
					if(SYMTAB.containsKey(arr[0]))
						SYMTAB.put(arr[0], Integer.toHexString(LC).toUpperCase());
					
					name = arr[0];
				} else if(arr[1].equals("END")) {
					if(arr.length == 3) {
						try {
							end = Integer.parseInt(arr[2], 16);
						} catch (NumberFormatException e) {
							if(SYMTAB.containsKey(arr[2]))
								end = Integer.parseInt(SYMTAB.get(arr[2]), 16);
							else {
								System.out.println("\n" + lineNum + ": Label \"" + arr[2] + "\" not declared. Aborted.");
								intFile.close();
								scan.close();
								(new File("intm_" + filename)).delete();
								System.exit(0);
							}
						}
						
						intFile.write(Integer.toHexString(LC).toUpperCase() + "\t" + arr[1] + "\t" + arr[2] + "\n");
					} else {
						end = start;
						
						intFile.write(Integer.toHexString(LC).toUpperCase() + "\t" + arr[1] + "\n");
					}
					
					length = LC - start;
					break;
				} else if(op.containsKey(arr[1])) {
					if(arr.length == 3)
						intFile.write(Integer.toHexString(LC).toUpperCase() + "\t" + arr[1] + "\t" + arr[2] + "\n");
					else
						intFile.write(Integer.toHexString(LC).toUpperCase() + "\t" + arr[1] + "\n");
					
					LC += 3;
				} else if(ps_op.contains(arr[1])) {
					intFile.write(Integer.toHexString(LC).toUpperCase() + "\t" + arr[1] + "\t" + arr[2] + "\n");
					
					switch(arr[1]) {
						case "WORD":
							LC += 3;
							break;
						case "BYTE":
							if(arr[2].charAt(0) == 'C')
								LC += arr[2].length() - 3;
							else if(arr[2].charAt(0) == 'X')
								LC += (arr[2].length() - 3) / 2;
							break;
						case "RESW":
							LC += 3 * Integer.parseInt(arr[2]);
							break;
						case "RESB":
							LC += Integer.parseInt(arr[2]);
							break;
					}
				} else {
					System.out.println("\n" + lineNum + ": Invalid opcode \"" + arr[1] + "\". Aborted.");
					intFile.close();
					scan.close();
					(new File("intm_" + filename)).delete();
					System.exit(0);
				}
				
				lineNum++;
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		
		intFile.close();
		
		displayPass1(filename);
		scan.close();
	}
	
	void pass2(String filename) throws IOException {
		File file = new File("intm_" + filename);
		FileWriter objFile = new FileWriter("obj_" + filename);
		FileWriter listFile = new FileWriter("list_" + filename);
		Scanner scan = new Scanner(file);
		int lineNum = 1;
		
		String startStr = Integer.toHexString(start).toUpperCase();
						
		int len = startStr.length();
		
		for(int i = 6; i > len; i--)
			startStr = "0" + startStr;
			
		String endStr = Integer.toHexString(end).toUpperCase();
						
		len = endStr.length();
		
		for(int i = 6; i > len; i--)
			endStr = "0" + endStr;
		
		String lengStr = Integer.toHexString(length).toUpperCase();
		
		len = lengStr.length();
		
		for(int i = 6; i > len; i--)
			lengStr = "0" + lengStr;
		
		for(int i = name.length(); i < 6; i++)
			name = name + " ";
		
		objFile.write("H^" + name + "^" + startStr + "^" + lengStr + "\n");
		
		int count = 0;
		String str1 = "";
		String startRec = "";
		
		while(scan.hasNextLine()) {
			try {
				String str = scan.nextLine();
				String[] arr = str.split("\t");
				
				String instr = "";
				
				if(op.containsKey(arr[1])) {
					if(arr.length == 3 && !SYMTAB.containsKey(arr[2].replace(",X", ""))) {
						System.out.println("\n" + lineNum + ": Label \"" + arr[2] + "\" not declared. Aborted.");
						objFile.close();
						listFile.close();
						scan.close();
						file.delete();
						(new File("list_" + filename)).delete();
						(new File("obj_" + filename)).delete();
						System.exit(0);
					} else if(arr.length == 3 && arr[2].contains(",X")) {
						instr = op.get(arr[1]);
						instr = instr.concat(
							Integer.toHexString(
								Integer.parseInt(
									SYMTAB.get(arr[2].replace(",X", "")), 16) + (int) Math.pow(2, 15)
							).toUpperCase()
						);
					} else {
						instr = op.get(arr[1]);
						instr = instr.equals("4C") ? instr.concat("0000") : instr.concat(SYMTAB.get(arr[2]));
					}
				}
				else {
					if(arr[1].equals("BYTE")) {
						if(arr[2].charAt(0) == 'X') {
							arr[2] = arr[2].substring(2, arr[2].length() - 1);
							instr = arr[2];
						}
						else if(arr[2].charAt(0) == 'C') {
							arr[2] = arr[2].substring(2, arr[2].length() - 1);
							
							char[] ch = arr[2].toCharArray();

							for (char c : ch)
								instr = instr.concat(String.format("%H", c));
						}
					} else if(arr[1].equals("WORD")) {
						instr = Integer.toHexString(Integer.parseInt(arr[2])).toUpperCase();
						
						len = instr.length();
						for(int i = 6; i > len; i--)
							instr = "0" + instr;
					}
				}
				
				listFile.write(arr[0] + "\t" + instr + "\n");
				
				lineNum++;
				
				if(instr.isEmpty() || count >= 30) {
					if(!str1.isEmpty()) {
						String recLen = Integer.toHexString(count).toUpperCase();
						if(recLen.length() == 1)
							recLen = "0" + recLen;
							
						objFile.write("T^"  + startRec + "^" + recLen + str1 + "\n");
						str1 = "";
						
						startRec = arr[0];
						
						len = startRec.length();
						for(int i = 6; i > len; i--)
							startRec = "0" + startRec;
					}
					
					count = 0;
				} 
				
				if(str1.isEmpty()) {
					startRec = arr[0];
					
					len = startRec.length();
					for(int i = 6; i > len; i--)
						startRec = "0" + startRec;
				}
				
				if(!instr.isEmpty()) {
					str1 = str1 + "^" + instr;
					count += instr.length() / 2;
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		
		objFile.write("E^" + endStr);
		
		listFile.close();
		objFile.close();
		
		displayPass2(filename);
		
		new File("list_" + filename).delete();
		file.delete();
		scan.close();
	}
	
	void displayPgm(String filename) throws IOException {
		int lineNum = 1;
		Scanner scan = new Scanner(new File(filename));
		
		System.out.println("\nYour SIC program is\n");
		while(scan.hasNextLine()) {
			System.out.print(lineNum);
			lineNum++;
			System.out.println(": " + scan.nextLine());
		}
		
		scan.close();
	}
	
	void displayPass1(String filename) throws IOException {
		int lineNum = 1;
		Scanner scan = new Scanner(new File("intm_" + filename));
		
		System.out.println("\nPass 1\n");
		while(scan.hasNextLine()) {
			System.out.print(lineNum);
			lineNum++;
			System.out.println(": " + scan.nextLine());
		}
		
		System.out.println("\nSymbol\tAddress");
		SYMTAB.forEach((key, value) ->
			System.out.println(key + "\t" + value));
		
		scan.close();
	}
	
	void displayPass2(String filename) throws IOException {
		Scanner scan = new Scanner(new File("list_" + filename));
		
		System.out.println("\nPass 2\n");
		
		while(scan.hasNextLine()) {
			System.out.println(scan.nextLine());
		}
		
		scan.close();
		
		scan = new Scanner(new File("obj_" + filename));
		
		System.out.println("\nObject code\n");
		
		while(scan.hasNextLine()) {
			System.out.println(scan.nextLine());
		}
		
		scan.close();
	}
	
	public static void main(String args[]) {
		SIC obj = new SIC();
		
		obj.initialise();
		
		System.out.print("Enter filename of SIC program: ");
		Scanner in = new Scanner(System.in);
		String filename = in.nextLine();
		
		try {
			obj.pass1(filename);
			obj.pass2(filename);
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
