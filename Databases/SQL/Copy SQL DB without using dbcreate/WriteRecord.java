import oracle.kv.KVStore;
import oracle.kv.KVStoreConfig;
import oracle.kv.KVStoreFactory;
import oracle.kv.Key;
import oracle.kv.Value;
import oracle.kv.ValueVersion;

class WriteRecord{

    public static void main(String args[]) {
        try {
		
			String storeName = "kvstore";
			String hostName = ". . .";
			String hostPort = "5000";
			
			String keyString = "myKey";
			String valueString = "newValue";
			
		    KVStore store;
						
			store = KVStoreFactory.getStore(new KVStoreConfig(storeName, hostName + ":" + hostPort));
            System.out.println("Store " + storeName + " at " + hostName + ":" + hostPort + " opened.");
			
			Key key = Key.createKey(keyString);
			Value value = Value.createValue(valueString.getBytes());
            store.put(key, value);
			System.out.println("Record <" + keyString + "," + valueString + "> saved.");
			
            store.close();
			System.out.println("Store " + storeName + " at " + hostName + ":" + hostPort + " closed.");
			
        }
		catch (RuntimeException e) {
            e.printStackTrace();
        }
    }
}
