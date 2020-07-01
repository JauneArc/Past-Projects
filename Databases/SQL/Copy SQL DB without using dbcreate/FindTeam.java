import oracle.kv.KVStoreFactory;
import oracle.kv.Key;
import oracle.kv.Value;
import oracle.kv.ValueVersion;
import oracle.kv.KeyValueVersion;
import oracle.kv.Direction;

import java.util.ArrayList;
import java.util.Iterator;

class ReadAll{

    public static void main(String args[]) {
        try {
		
			String storeName = "kvstore";
			String hostName = ". . .";
			String hostPort = "5000";
			int T1 =0;
			int T2 =0;
			String TeamName;
		    KVStore store;
						
			store = KVStoreFactory.getStore(new KVStoreConfig(storeName, hostName + ":" + hostPort));
            System.out.println("Store " + storeName + " at " + hostName + ":" + hostPort + " opened.");

			Iterator<KeyValueVersion> it = store.storeIterator(Direction.UNORDERED,0);
			//System.out.println("In StoreIterator Method: Print Everything");
			while (it.hasNext() )
			{
				KeyValueVersion kvvi = it.next();	
				TeamName = kvvi.getValue().getValue;
				if(TeamName == "TerribleTeam")
					T2++;
				else if(TeamName == "Killer Team")
						T1++;
			}
            store.close();
			System.out.println("Killer Team number of players: " + T1);
			System.out.println("Terrible Team number of players :" + T2);
			System.out.println("Store " + storeName + " at " + hostName + ":" + hostPort + " closed.");
			
        }
		catch (RuntimeException e) {
            e.printStackTrace();
        }
    }
}
