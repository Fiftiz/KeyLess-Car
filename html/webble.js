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
    const checkboxAutoRun = document.getElementById('checkboxAutoLockRun');
    const checkboxDiagMode = document.getElementById('checkboxDiag');
    const bleStateContainer = document.getElementById('bleState');
    const statusButton = document.getElementById('statusButton');
    const statusVoltage = document.getElementById('voltage');

    //Define BLE Device Specs
    var deviceName ='KeyLess Car';
    var bleService = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
    var unlockCharacteristic = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
    var autoCharacteristic = 'beb5483f-36e2-4688-b7f5-ea07361b26a8';
    var autoRunCharacteristic = 'beb5483a-36e3-4688-b7f5-ea07361b26a8';
    var DiagCharacteristic = 'beb5483c-36e5-4688-b7f5-ea07361b26a8';
    var PinCharacteristic = 'beb5483d-36e6-4687-b7f5-ea07361b26a8';

    //Global Variables to Handle Bluetooth
    var bleServer;
    var bleServiceFound;
    var unlockCharacteristics;
    var autoCharacteristics;
    var autoRunCharacteristics;
    var DiagCharacteristics;
    //var BattCharacteristics;
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
    checkboxAutoRun.addEventListener('change', autoLockRun);
    checkboxDiagMode.addEventListener('change', DiagMode);
    
    // Write to the ESP32 Characteristic
    //onButton.addEventListener('click', () => writeOnCharact(1));
    //offButton.addEventListener('click', () => writeOnCharact(0));

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
        statusButtonConnection.textContent = 'Connexion...';

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
            service.getCharacteristic(unlockCharacteristic),
            service.getCharacteristic(autoCharacteristic),
            service.getCharacteristic(autoRunCharacteristic),
            service.getCharacteristic(DiagCharacteristic),
            //service.getCharacteristic(BattCharacteristic)
            ]);
        })
        .then(characteristics => {
            [unlockCharacteristics, autoCharacteristics, autoRunCharacteristics, DiagCharacteristics] = characteristics;
            
            
            unlockCharacteristics.addEventListener('characteristicvaluechanged', handleCharactChange);
            unlockCharacteristics.startNotifications().catch(error => {
                console.error('Error starting notifications for unlockCharacteristic:', error);
            });
            console.log("Unlock / Lock Characteristic discovered:", unlockCharacteristics.uuid);

            autoCharacteristics.addEventListener('characteristicvaluechanged', handleCharactChangeAuto);
            autoCharacteristics.startNotifications().catch(error => {
                console.error('Error starting notifications for autoCharacteristics:', error);
            });
            console.log("AutoUnlock Characteristic discovered:", autoCharacteristics.uuid);

            autoRunCharacteristics.addEventListener('characteristicvaluechanged', handleCharactChangeAutoRun);
            autoRunCharacteristics.startNotifications().catch(error => {
                console.error('Error starting notifications for autoRunCharacteristics:', error);
            });
            console.log("Auto Lock Run Characteristic discovered:", autoRunCharacteristics.uuid);

            DiagCharacteristics.addEventListener('characteristicvaluechanged', handleCharactChangeDiagMode);
            DiagCharacteristics.startNotifications().catch(error => {
                console.error('Error starting notifications for DiagCharacteristics:', error);
            });
            console.log("Daig Mode Characteristic discovered:", DiagCharacteristics.uuid);

            
            /*BattCharacteristics.addEventListener('characteristicvaluechanged', handleCharactChangeBatt);
            BattCharacteristics.startNotifications().catch(error => {
                console.error('Error starting notifications for BattCharacteristic:', error);
            });
            console.log("Battery Characteristic discovered:", BattCharacteristics.uuid);*/



            console.log("Notifications Started.");
            return Promise.all([
                unlockCharacteristics.readValue(),
                autoCharacteristics.readValue(),
                autoRunCharacteristics.readValue(),
                DiagCharacteristics.readValue()
                //BattCharacteristics.readValue()
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

    function handleCharactChange(event){
        const newValueReceived = new TextDecoder().decode(event.target.value);
        console.log("Lock / Unlock Statue changed: ", newValueReceived);
        if (newValueReceived == 1){
            statusButton.textContent = 'UNLOCK';
            isOpen = true;
        }
        if (newValueReceived == 0){
            statusButton.textContent = 'LOCK';
            isOpen = false;
        }
    }

    function handleCharactChangeAuto(event){
        const newValueReceivedAuto = new TextDecoder().decode(event.target.value);
        console.log("Auto Unlock Statue changed: ", newValueReceivedAuto);
        if (newValueReceivedAuto == 1){
            checkboxAuto.checked = true;
        }
        if (newValueReceivedAuto == 0){
            checkboxAuto.checked = false;
        }
    }

    function handleCharactChangeAutoRun(event){
        const newValueReceivedAutoRun = new TextDecoder().decode(event.target.value);
        console.log("Auto Run State changed: ", newValueReceivedAutoRun);
        if (newValueReceivedAutoRun == 1){
            checkboxAutoRun.checked = true;
        }
        if (newValueReceivedAutoRun == 0){
            checkboxAutoRun.checked = false;
        }
    }

    function handleCharactChangeDiagMode(event){
        const newValueReceivedDiag = new TextDecoder().decode(event.target.value);
        console.log("Daig Mode Statue changed: ", newValueReceivedDiag);
        /*if (newValueReceivedDiag == 1){
            checkboxDiagMode.checked = true;
        }
        if (newValueReceivedDiag == 0){
            checkboxDiagMode.checked = false;
        }*/
        statusVoltage.innerHTML = newValueReceivedDiag;
    }

    /*function handleCharactChangeBatt(event){
        const newValueReceivedBatt = new TextDecoder().decode(event.target.value);
        console.log("Voltage value changed: ", newValueReceivedBatt);
            statusVoltage.innerHTML = newValueReceivedBatt;
    }*/

    function writeOnCharact(value){
        if (bleServer && bleServer.connected) {
            bleServiceFound.getCharacteristic(unlockCharacteristic)
            .then(characteristic => {
                console.log("Found the  unlockCharacteristic: ", characteristic.uuid);
                const data = new Uint8Array([value]);
                return characteristic.writeValue(data);
            })
            .then(() => {
                console.log("Value written to unlockCharacteristic:", value);
            })
            .catch(error => {
                console.error("Error writing to the unlockCharacteristic: ", error);
            });
        } else {
            console.error ("Bluetooth is not connected. Cannot write to characteristic.")
            window.alert("Bluetooth is not connected. Cannot write to characteristic. \n Connect to BLE first!")
        }
    }

    function writeOnAutoCharact(value){
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

    function writeOnAutoRunCharact(value){
        if (bleServer && bleServer.connected) {
            bleServiceFound.getCharacteristic(autoRunCharacteristic)
            .then(characteristic => {
                console.log("Found the  autoRunCharacteristic: ", characteristic.uuid);
                const data = new Uint8Array([value]);
                return characteristic.writeValue(data);
            })
            .then(() => {
                console.log("Value written to autoRunCharacteristic:", value);
            })
            .catch(error => {
                console.error("Error writing to the autoRunCharacteristic: ", error);
            });
        } else {
            console.error ("Bluetooth is not connected. Cannot write to characteristic.")
            window.alert("Bluetooth is not connected. Cannot write to characteristic. \n Connect to BLE first!")
        }
    }

    function writeOnDiagCharact(value){
        if (bleServer && bleServer.connected) {
            bleServiceFound.getCharacteristic(DiagCharacteristic)
            .then(characteristic => {
                console.log("Found the  DiagCharacteristic: ", characteristic.uuid);
                const data = new Uint8Array([value]);
                return characteristic.writeValue(data);
            })
            .then(() => {
                console.log("Value written to DiagCharacteristic:", value);
            })
            .catch(error => {
                console.error("Error writing to the DiagCharacteristic: ", error);
            });
        } else {
            console.error ("Bluetooth is not connected. Cannot write to characteristic.")
            window.alert("Bluetooth is not connected. Cannot write to characteristic. \n Connect to BLE first!")
        }
    }

    function writePinCharact(){
        if (bleServer && bleServer.connected) {
            const newPassword = document.getElementById('Passinput').value
            if (newPassword.length == 6) {
                bleServiceFound.getCharacteristic(PinCharacteristic)
            
                .then(characteristic => {
                    console.log("Found the  PinCharacteristic: ", characteristic.uuid);
                    const encoder = new TextEncoder();
                    return characteristic.writeValue(encoder.encode(newPassword));
                })
                .then(() => {
                    console.log("Password changed successfully");
                    window.alert("Password changed successfully, Reboot in 2 seconds")
                })
                .catch(error => {
                    console.error("Error writing to the PinCharacteristic: ", error);
                });
            } else {
                console.error ('Le mot de passe doit comporter exactement 6 chiffres.')
                window.alert('Le mot de passe doit comporter exactement 6 chiffres.')
            }

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
            writeOnCharact(1);
        } else {
            statusButton.textContent = 'Locking...';
            writeOnCharact(0);
        }
    }

    function autoLockUnlock(){
        console.log(`Auto Unlock/Lock ${checkboxAuto.checked ? 'activated' : 'deactivated'}.`);
        const value = checkboxAuto.checked ? 1 : 0;
        if (value == 1){
            writeOnAutoCharact(1);
        }
        if (value == 0){
            writeOnAutoCharact(0);
        }
    }

    function autoLockRun(){
        console.log(`Auto Lock Run ${checkboxAutoRun.checked ? 'activated' : 'deactivated'}.`);
        const value = checkboxAutoRun.checked ? 1 : 0;
        if (value == 1){
            writeOnAutoRunCharact(1);
        }
        if (value == 0){
            writeOnAutoRunCharact(0);
        }
    }

    function DiagMode(){
        console.log(`Auto Lock Run ${checkboxDiagMode.checked ? 'activated' : 'deactivated'}.`);
        const value = checkboxDiagMode.checked ? 1 : 0;
        if (value == 1){
            writeOnDiagCharact(1);
        }
        if (value == 0){
            writeOnDiagCharact(0);
        }
    }
 
