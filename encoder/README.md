# Программа для работы с датчиком угла поворота

Сборка:
```  
gcc -o encoder encoder.c rotary_encoder.c -lpigpio  
```

Запуск:  
sudo ./encoder [-h][-q]  
-h - описание работы  
-q - тихий режим, выводится только сообщение при срабатывании датчика  

Выходные данные: 
В случае поворота энкодера по часовой стрелке выходное значение инкрементируется  
При повороте против часовой стрелки - декрементируется.  

Пример: 
```
sudo ./encoder -q  //Вывод значений в "тихом режиме"
```
