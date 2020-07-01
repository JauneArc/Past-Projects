import oracle.kv.KVStore;
import oracle.kv.KVStoreConfig;
import oracle.kv.KVStoreFactory;
import oracle.kv.Key;
import oracle.kv.Value;
import oracle.kv.ValueVersion;

class DeleteRecord{

    public static void main(String args[]) {
        try {
		
			String storeName = "kvstore";
			String hostName = ". . .";
			String hostPort = "5000";
			
			String keyString = "Jack";
			ArrayList<String> majorList = new ArrayList<String>();
		    ArrayList<String> minorList = new ArrayList<String>();
			Key key2;
			Value value;
			String valueString; 
		    KVStore store;
						
			store = KVStoreFactory.getStore(new KVStoreConfig(storeName, hostName + ":" + hostPort));
            System.out.println("Store " + storeName + " at " + hostName + ":" + hostPort + " opened.");
			
			//delete Jack for Killer Team
			Key key = Key.createKey(keyString);
            Boolean b = store.delete(key);
			if ( b )
			  System.out.println("Record <" + keyString + ", ... > deleted.");
			
			//insert Jack for Terrible Team
			majorList.add("Jack");
			majorList.add("Ripper");
			majorList.add("D-O-B");
			majorList.add("British");
			minorList.add("Team");
			valueString = "Terrible Team";
			key2 = Key.createKey(majorList, minorList);			
			value = Value.createValue(valueString.getBytes());
			store.put(key2, value);
			
			
            store.close();
			System.out.println("Store " + storeName + " at " + hostName + ":" + hostPort + " closed.");
			
        }
		catch (RuntimeException e) {
            e.printStackTrace();
        }
    }
}
