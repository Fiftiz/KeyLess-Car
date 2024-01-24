/*********
  THIROUX Yannis 
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  ©2023 THIROUX Yannis, tous droits réservés.
*********/

  //window.addEventListener('load', document.getElementById('valueSent'));
    // DOM Elements
    const connectButton = document.getElementById('connectBleButton');
    const disconnectButton = document.getElementById('disconnectBleButton');
    const checkboxAuto = document.getElementById('checkboxAuto');
    const bleStateContainer = document.getElementById('bleState');
    const statusButton = document.getElementById('statusButton');

    //Define BLE Device Specs
    var deviceName ='KeyLess Car';
    var bleService = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
    var pCharacteristic = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
    var autoCharacteristic = 'beb5483f-36e2-4688-b7f5-ea07361b26a8';

    //Global Variables to Handle Bluetooth
    var bleServer;
    var bleServiceFound;
    var pCharacteristicFound;
    var autoCharacteristicFound;
    let isOpen = false;
    let blinkProgress = false;
    // Connect Button (search for BLE Devices only if BLE is available)
    connectButton.addEventListener('click', (event) => {
        if (isWebBluetoothEnabled()){
            connectToDevice();
        }
    });

    // Disconnect Button
    disconnectButton.addEventListener('click', disconnectDevice);
    // Auto Lock Unlock Checkbox
    checkboxAuto.addEventListener('change', autoLockUnlock);

    // Write to the ESP32 Characteristic
    //onButton.addEventListener('click', () => writeOnCharacteristic(1));
    //offButton.addEventListener('click', () => writeOnCharacteristic(0));

    // Check if BLE is available in your Browser
    function isWebBluetoothEnabled() {
        if (!navigator.bluetooth) {
            console.log("Web Bluetooth API is not available in this browser!");
            bleStateContainer.innerHTML = "Web Bluetooth API is not available in this browser!";
            return false
        }
        console.log('Web Bluetooth API supported in this browser.');
        return true
    }

    // Connect to BLE Device and Enable Notifications
    function connectToDevice(){
        console.log('Initializing Bluetooth...');
        const statusButtonConnection = document.getElementById('statusButton');
        statusButtonConnection.textContent = 'Chargement...';

        navigator.bluetooth.requestDevice({
            filters: [{name: deviceName}],
            optionalServices: [bleService]
        })

        .then(device => {
            console.log('Device Selected:', device.name);
            device.addEventListener('gattservicedisconnected', onDisconnected);
            return device.gatt.connect();
        })
        .then(gattServer =>{
            bleServer = gattServer;
            console.log("Connected to GATT Server");
            return bleServer.getPrimaryService(bleService);
        })
        .then(service => {
            bleServiceFound = service;
            console.log("Service discovered:", service.uuid);
            return Promise.all([
            service.getCharacteristic(pCharacteristic),
            service.getCharacteristic(autoCharacteristic)
            ]);
        })
        .then(characteristics => {
            [pCharacteristics, autoCharacteristics] = characteristics;
            
            
            pCharacteristics.addEventListener('characteristicvaluechanged', handleCharacteristicChange);
            pCharacteristics.startNotifications();
            console.log("Characteristic discovered:", pCharacteristics.uuid);

            autoCharacteristics.addEventListener('characteristicvaluechanged', handleCharacteristicChangeAuto);
            autoCharacteristics.startNotifications();
            console.log("Characteristic discovered:", autoCharacteristics.uuid);

            console.log("Notifications Started.");
            return Promise.all([
                pCharacteristics.readValue(),
                autoCharacteristics.readValue()
            ]);
        })
        .then(value => {
            bleStateContainer.innerHTML = 'Connected to Car';
            bleStateContainer.style.color = "#ffA500";
            var button = document.querySelector('.statusButton');
            button.classList.add('statusButton-connected');
        })
        .catch(error => {
            console.log('Error: ', error);
            bleStateContainer.innerHTML = 'Error! Try to reconnect';
            bleStateContainer.style.color = "#ffA500";
        })
    }

    function onDisconnected(event){
        console.log('Device Disconnected:', event.target.device.name);
        bleStateContainer.innerHTML = "Disconnected";
        bleStateContainer.style.color = "#bebebe";
        var button = document.querySelector('.statusButton');
        button.classList.remove('statusButton-connected');
        connectToDevice();
    }

    function handleCharacteristicChange(event){
        const newValueReceived = new TextDecoder().decode(event.target.value);
        console.log("Characteristic value changed: ", newValueReceived);
        if (newValueReceived == 1){
            statusButton.textContent = 'UNLOCK';
            isOpen = true;
        }
        if (newValueReceived == 0){
            statusButton.textContent = 'LOCK';
            isOpen = false;
        }
    }

    function handleCharacteristicChangeAuto(event){
        const newValueReceivedAuto = new TextDecoder().decode(event.target.value);
        console.log("Characteristic value changed: ", newValueReceivedAuto);
        if (newValueReceivedAuto == 1){
            checkboxAuto.checked = true;
        }
        if (newValueReceivedAuto == 0){
            checkboxAuto.checked = false;
        }
    }

    function writeOnCharacteristic(value){
        if (bleServer && bleServer.connected) {
            bleServiceFound.getCharacteristic(pCharacteristic)
            .then(characteristic => {
                console.log("Found the  pcharacteristic: ", characteristic.uuid);
                const data = new Uint8Array([value]);
                return characteristic.writeValue(data);
            })
            .then(() => {
                console.log("Value written to pcharacteristic:", value);
            })
            .catch(error => {
                console.error("Error writing to the pcharacteristic: ", error);
            });
        } else {
            console.error ("Bluetooth is not connected. Cannot write to characteristic.")
            window.alert("Bluetooth is not connected. Cannot write to characteristic. \n Connect to BLE first!")
        }
    }

    function writeOnAutoCharacteristic(value){
        if (bleServer && bleServer.connected) {
            bleServiceFound.getCharacteristic(autoCharacteristic)
            .then(characteristic => {
                console.log("Found the  autocharacteristic: ", characteristic.uuid);
                const data = new Uint8Array([value]);
                return characteristic.writeValue(data);
            })
            .then(() => {
                console.log("Value written to autocharacteristic:", value);
            })
            .catch(error => {
                console.error("Error writing to the autocharacteristic: ", error);
            });
        } else {
            console.error ("Bluetooth is not connected. Cannot write to characteristic.")
            window.alert("Bluetooth is not connected. Cannot write to characteristic. \n Connect to BLE first!")
        }
    }


    function disconnectDevice() {
        console.log("Disconnect Device.");
        if (bleServer && bleServer.connected) {
            bleServer.disconnect();
            bleStateContainer.innerHTML = "Device Disconnected";
            bleStateContainer.style.color = "#bebebe";
            var button = document.querySelector('.statusButton');
            button.classList.remove('statusButton-connected');

        } else {
            // Throw an error if Bluetooth is not connected
            console.error("Bluetooth is not connected.");
            window.alert("Bluetooth is not connected.")
        }
    }

    /*function CharacteristicConv(value){
        const statusButton = document.getElementById('statusButton');
        if (value == 1){
            statusButton.textContent = 'UNLOCK';
            isOpen = true;
        }
        if (value == 0){
            statusButton.textContent = 'LOCK';
            isOpen = false;
        }
    }

    function CharacteristicConvAuto(value){
        if (value == 1){
            checkboxAuto.checked = true;
        }
        if (value == 0){
            checkboxAuto.checked = false;
        }
    }*/

    function toggleStatus() {
        isOpen = !isOpen;

        const statusButton = document.getElementById('statusButton');

        var button = document.querySelector('.statusButton');
        button.classList.add('blink-animation');
        // Arrêtez le clignotement après 3 secondes
        setTimeout(function () {button.classList.remove('blink-animation');}, 3000);
        
        if (isOpen) {
            statusButton.textContent = 'Unlocking...';
            writeOnCharacteristic(1);
        } else {
            statusButton.textContent = 'Locking...';
            writeOnCharacteristic(0);
        }
    }

    function autoLockUnlock(){
        console.log(`Auto Unlock/Lock ${checkboxAuto.checked ? 'activated' : 'deactivated'}.`);
        const value = checkboxAuto.checked ? 1 : 0;
        if (value == 1){
            writeOnAutoCharacteristic(1);
        }
        if (value == 0){
            writeOnAutoCharacteristic(0);
        }
    }
