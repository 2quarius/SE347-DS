# MapReduce
## Part I Map/Reduce input and output
### doMap()
```java
class Mapper {
    /*
     * @param jobName the name of the MapReduce job
     * @param mapTask which map task this is
     * @param inFile file name (if in same dir, it's also the file path)
     * @param nReduce the number of reduce task that will be run ("R" in the paper)
     * @param mapF the user-defined map function
     */
    public static void doMap(String jobName, int mapTask, String inFile, int nReduce, MapFunc mapF) {
        try {
                  /**
                   * read from target file and do map
                   */
                  InputStream inputStream = new FileInputStream(inFile);
                  byte[] content = new byte[inputStream.available()];
                  inputStream.read(content);
                  List<KeyValue> keyValueList = mapF.map(inFile, new String(content));
                  inputStream.close();
      
                  /**
                   * partition:
                   * first create nReduce intermediate files
                   * second traverse key/value list to write into corresponding intermediate files
                   */
                  List<OutputStream> intermediateFiles = new ArrayList<>();
                  List<List<KeyValue>> intermediateContents = new ArrayList<>();
                  for(int r = 0; r < nReduce; r++) {
                      OutputStream outputStream = new FileOutputStream(Utils.reduceName(jobName, mapTask, r));
                      intermediateFiles.add(outputStream);
                      intermediateContents.add(new ArrayList<>());
                  }
                  for(KeyValue kv : keyValueList) {
                      intermediateContents.get(hashCode(kv.key) % nReduce).add(kv);
                  }
                  for(int r = 0; r < nReduce; r++) {
                      intermediateFiles.get(r).write(JSON.toJSONString(intermediateContents.get(r)).getBytes());
                      intermediateFiles.get(r).close();
                  }
      
              } catch (IOException e) {
                  e.printStackTrace();
              }
    }
}
```
### doReduce()
```java
class Reducer {
    /* 	
     * @param jobName the name of the whole MapReduce job
     * @param reduceTask which reduce task this is
     * @param outFile write the output here
     * @param nMap the number of map tasks that were run ("M" in the paper)
     * @param reduceF user-defined reduce function
     */
    public static void doReduce(String jobName, int reduceTask, String outFile, int nMap, ReduceFunc reduceF) {
        try {
            // read from each intermediate file and combine distinct key
            Map<String, List<String>> keyValues = new HashMap<>();
            for (int m = 0; m < nMap; m++) {
                // read file and parse JSON to array
                InputStream inputStream = new FileInputStream(Utils.reduceName(jobName, m, reduceTask));
                byte[] content = new byte[inputStream.available()];
                inputStream.read(content);
                List<KeyValue> kvContent = JSON.parseArray(new String(content), KeyValue.class);
                // traverse array to fill map
                for(KeyValue kv : kvContent) {
                    if(keyValues.containsKey(kv.key)) {
                        keyValues.get(kv.key).add(kv.value);
                    }
                    else {
                        List<String> values = new ArrayList<>();
                        values.add(kv.value);
                        keyValues.put(kv.key,values);
                    }
                }
                inputStream.close();
            }
            // do reduce on each distinct key and write to target file
            JSONObject jsonObject = new JSONObject();
            for(Map.Entry<String, List<String>> entry : keyValues.entrySet()) {
                String value = reduceF.reduce(entry.getKey(), entry.getValue().toArray(new String[0]));
                jsonObject.put(entry.getKey(), value);
            }
            OutputStream outputStream = new FileOutputStream(outFile);
            outputStream.write(jsonObject.toString().getBytes());
            outputStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
```
## Part II Single-worker word count