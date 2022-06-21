const char WifiSetupPage[] PROGMEM = R"=====(
<html>
    <head>
        <!-- <link rel="shortcut icon" href="/data/favicon.ico" type="image/x-icon" />  -->
        <meta content="width=device-width, initial-scale=1" name="viewport" />
        <title>Health Hygiene Monitoring System</title>
    </head>
    <style>

        body {
            /* height: 100vh;
            width: 100vw; */
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 0rem;
            margin: 0rem;
        }

        .healthHygienePage {
            min-height: 20rem;
            box-shadow: 0 0.3rem 0.7rem #a8cc8d;
            margin: 1rem 0rem;
            z-index: 1;
        }

        .hhmPageLogodiv {
            height: 4.5rem;
            width: 100%;
            margin: 1rem 0rem;
            display: flex;
            justify-content: center;
        }

        .hhmSetupParams {
            margin: 0rem 2rem 1rem 2rem;
        }

        .commonCssForInputDeviceSetupPage {
            width: 100%;
            margin-right: 2rem;
            border: 1px solid #a8cc8d;
            border-radius: 5px;
            height: 2rem;
            font-size: 1rem;
        }

        .cssForSelectInputDeviceSetupPage {
            width: 100%;
        }

        .labelsForInputDeviceSetup {
            font-size: 1.1rem;
        }

        select {
            width: 100%;
            height: 2.3rem;
            font-size: 1rem;
            border: 1px solid #a8cc8d;
        }

        #saveDetailsButton {
            background-color: #a8cc8d;
            font-size: large;
            color: white;
            height: 2.5rem;
            width: 100%;
            border-radius: 5px;
            margin-bottom: 1rem;
        }

        #togglePasswordButton {
            background-color: #a8cc8d;
            color: white;
            border: 1px solid black;
            border-radius: 5px;
            padding: 5px;
        }

        p {
            color: red;
            margin: 0rem;
        }

        .container {
            display: flex;
            align-items: center;
        }

        .container i {
            margin: 0px;
            cursor: pointer;
        }

        @media(max-width: 500px) {

            body {
                padding: 0rem;
                margin: 1rem 0rem;
            }

            .healthHygienePage {
                margin: 0rem 2rem;
                box-shadow: 0 0.3rem 0.7rem #a8cc8d; 
            }

            .hhmPageLogodiv {
                height: 3.5rem;
                width: 100%;
                margin: 1rem 0rem;
                justify-content: center;
            }

            .hhmSetupParams {
                margin: 0rem 2rem 1rem 2rem;                
            }

            .commonCssForInputDeviceSetupPage {
                width: 100%;
                margin-right: 2rem;
                border: 1px solid #a8cc8d;
                border-radius: 5px;
                height: 2rem;
                font-size: 1rem;
            }

            .cssForSelectInputDeviceSetupPage {
                width: 100%;
            }

            .labelsForInputDeviceSetup {
                font-size: 1.1rem;
            }

            select {
                width: 100%;
                height: 2.3rem;
                font-size: 1rem;
                border: 1px solid #a8cc8d;
            }

            #saveDetailsButton {
                font-size: large;
                color: white;
                height: 2.5rem;
                width: 100%;
                border-radius: 5px;
                margin-bottom: 0.5rem;    
            }
        }

        @media(max-width: 320px) {

            body {
                padding: 0rem;
                margin: 1rem 0rem;
            }

            .healthHygienePage {
                margin: 0rem 2rem;
                box-shadow: 0 0.3rem 0.7rem #a8cc8d; 
            }

            .hhmPageLogodiv {
                height: 3.5rem;
                width: 100%;
                margin: 1rem 0rem;
                justify-content: center;
            }

           .hhmSetupParams {
                margin: 0rem 2rem 1rem 2rem;                
            }

            .commonCssForInputDeviceSetupPage {
                width: 100%;
                margin-right: 2rem;
                border: 1px solid #a8cc8d;
                border-radius: 5px;
                height: 2rem;
                font-size: 1rem;
            }

            .cssForSelectInputDeviceSetupPage {
                width: 100%;
            }

            .labelsForInputDeviceSetup {
                font-size: 1.1rem;
            }

            select {
                width: 100%;
                height: 2.3rem;
                font-size: 1rem;
                border: 1px solid #a8cc8d;
            }

            #saveDetailsButton {
                font-size: large;
                color: white;
                height: 2.5rem;
                width: 100%;
                border-radius: 5px;
                margin-bottom: 0.5rem;    
            }
        }

        #loader {
            border: 16px solid #f3f3f3;
            border-radius: 50%;
            border-top: 16px solid #a8cc8d;
            width: 5rem;
            height: 5rem;
            -webkit-animation: spin 2s linear infinite; /* Safari */
            animation: spin 2s linear infinite;
            z-index: 9999;
            position: fixed;
            top: 40%;
            left: 45%; 
            transform: translate(-40%, -40%);
        }

        /* Safari */
        @keyframes spin {
        0% { -webkit-transform: rotate(0deg); }
        100% { -webkit-transform: rotate(360deg); }
        }

        @keyframes spin {
        0% { transform: rotate(0deg); }
        100% { transform: rotate(360deg); }
        }

        #blackfilm {
            height: 100%;
            width: 100%;
            background-color: rgba(0,0,0, 0.5);
            position: fixed;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            margin: auto;
            z-index: 999;
        }
          

    </style>

    <body>
        <div id="blackfilm"></div>    
        <div id="loader"></div>
		<div class="healthHygienePage">
            <div class="hhmPageLogodiv">
                <img src="vilisoLogoNew.jpg" alt="Gandhvedh">
            </div>
			<form method="post" class="hhmsetuppage">
                <div class="hhmSetupParams">
                    <label for="SSID">
                        <span class="labelsForInputDeviceSetup">WiFi:</span>
                        <select type="text" id="SSID" class="select-field"  disabled>
                            <!--<option id="newfile" value="newfile">New file</option>-->
                        </select>
                    </label>
                </div>
                <div class="hhmSetupParams">
                    <label for="Password">
                        <span class="labelsForInputDeviceSetup">WiFi Password:</span>
                        <div class="container">
                            <input type="password" id="Password" class="commonCssForInputDeviceSetupPage"
                                required>
                            <div id="togglePasswordButton" onclick="showHidePassword()">SHOW</div>
                        </div>
                        <p id="invalidPassword"></p>
                    </label>
                </div>
                <div class="hhmSetupParams">
                    <label for="DeviceName">
                        <span class="labelsForInputDeviceSetup">Device Name:</span>
                        <input type="text" id="DeviceName" class="commonCssForInputDeviceSetupPage"
                                required>
                        <p id="invalidDeviceName"></p>
                    </label>
                </div>
                <div class="hhmSetupParams">
                    <label for="EmailID">
                        <span class="labelsForInputDeviceSetup">Email ID:</span>
                        <input type="email" id="EmailID" class="commonCssForInputDeviceSetupPage"
                                oncopy="return false" onpaste="return false" required>
                        <p id="invalidEmailID"></p>
                    </label>
                </div>
                <div class="hhmSetupParams">
                    <label for="ReEnterEmailID">
                        <span class="labelsForInputDeviceSetup">Re-enter Email ID:</span>
                        <input type="email" id="ReEnterEmailID" class="commonCssForInputDeviceSetupPage"
                                oncopy="return false" onpaste="return false" required>
                        <p id="invalidReEnterEmailID"></p>
                    </label>
                </div>
                <div class="hhmSetupParams">
                    <label for="Pincode">
                        <span class="labelsForInputDeviceSetup">Pincode:</span>
                        <input type="text" id="Pincode" class="commonCssForInputDeviceSetupPage"
                        required>
                    </label>
                </div>
                <div class="hhmSetupParams">
                    <label for="RoomType">
                            <span class="labelsForInputDeviceSetup">Room Type:</span>
                            <!-- <input type = "text" id="RoomType" class="commonCssForInputDeviceSetupPage"> -->
                            <select id="RoomType">
                                <option value="Hall">Hall</option>
                                <option value="Bedroom">Bedroom</option>
                                <option value="Kitchen">Kitchen</option>
                                <option value="Lab">Lab</option>
                                <option value="Toilet">Toilet</option>
                            </select>
                    </label>
                </div>
                <div class="hhmSetupParams">
                    <button type="button" id="saveDetailsButton" onclick="saveSSIDAndDeviceDetails()">SAVE</button>
                </div>    
			</form>
        </div>         
    </body>
</html>

<script>

    window.onload = function () {
        console.log("OnLoad called.");
        document.getElementById('loader').style.display = "none";
        document.getElementById('blackfilm').style.display = "none";

        getSSIDList();
    }

    function getSSIDList () {
        // console.log("getSSIDList called.");

        let select = document.getElementById("SSID");
		let length = select.options.length;

        // console.log("Options Length: ");
        // console.log(length);

        select.disabled = true;
        let xhttp;

        // Clear the options list if there is anything
        for(i = 0; i < length; i++) {
            select.options[0] = null;
        }

        // Show text 'Loading' while SSID list is populated
        let SSIDoption = document.createElement("option");
        SSIDoption.text = "Loading...";
        document.getElementById("SSID").add(SSIDoption);

        let URL = "/getSSIDList";
        xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {

            // console.log("Http Request Ready State: ");
            // console.log(this.readyState);
            // console.log("Http Request Status: ");
            // console.log(this.status);

            if (this.readyState == 4 && this.status == 200) {

                // console.log("Received SSIDs from Server.");
                // console.log("Response Text: ");
                // console.log(this.responseText);

                let SSIDSelect = document.getElementById("SSID");
                let SSIDArray = this.responseText.split(",");

                // console.log("SSID Split Array Length: ");
                // console.log(SSIDArray.length);

                SSIDSelect.options[0] = null;

                for (let i=0; i <= SSIDArray.length-1; i++) {

                    if (SSIDArray[i] != "") {
                        let SSIDoption = document.createElement("option");
                        SSIDoption.text = SSIDArray[i];
                        SSIDSelect.add(SSIDoption);
                    }
                }

                if (SSIDSelect.length != 0) {
                    SSIDSelect.selectedIndex = 0;
                }

                SSIDSelect.disabled = false;
            }
        };
        xhttp.open("GET", URL, true);
        xhttp.send();

    }    

    function saveSSIDAndDeviceDetails (event) {

        document.getElementById('invalidDeviceName').style.display = "none";
        document.getElementById('invalidEmailID').style.display = "none";
        document.getElementById('invalidReEnterEmailID').style.display = "none";
        document.getElementById('invalidPassword').style.display = "none";
        document.getElementById('loader').style.display = "none";
        document.getElementById('blackfilm').style.display = "none";

        console.log("saveSSIDAndDeviceDetails called.");

        let SSID = document.getElementById("SSID").value;
		let Password = document.getElementById("Password").value;
		let DeviceName = document.getElementById("DeviceName").value;
		let EmailID = document.getElementById("EmailID").value;
		let ReEnterEmailID = document.getElementById("ReEnterEmailID").value;
		let Pincode = document.getElementById("Pincode").value;
        let RoomType = document.getElementById("RoomType").value;

        let validEmailRegex = /^(([^<>()\[\]\.,;:\s@\"]+(\.[^<>()\[\]\.,;:\s@\"]+)*)|(\".+\"))@(([^<>()[\]\.,;:\s@\"]+\.)+[^<>()[\]\.,;:\s@\"]{2,})$/;
        if( validEmailRegex.test(EmailID) == true && validEmailRegex.test(ReEnterEmailID) == true && DeviceName.length > 3 && Password.length > 7 && EmailID == ReEnterEmailID) {
            // console.log("True");

            console.log("SSID: " + SSID);
            console.log("Pass: " + Password);
            console.log("DeviceName: " + DeviceName);        
            console.log("Email: " + EmailID);
            console.log("ReEnterEmailID: " + ReEnterEmailID);
            console.log("Pincode: " + Pincode);        
            console.log("RoomType: " + RoomType);
                      
            let urlPath = "saveSSIDAndDeviceDetails?"; 
            let urlSsid = SSID;
            let urlPassword = Password;
            let urlDeviceName = DeviceName;
            let urlEmailID = EmailID;
            let urlReEnterEmailID = ReEnterEmailID;
            let urlPincode = Pincode;
            let urlRoomType = RoomType;
            
            let encodedSSID = encodeURIComponent(urlSsid);
            let encodedPassword = encodeURIComponent(urlPassword);
            let encodedDeviceName = encodeURIComponent(urlDeviceName);
            let encodedEmailID = encodeURIComponent(urlEmailID);
            let encodedReEnterEmailID = encodeURIComponent(urlReEnterEmailID);
            let encodedPincode = encodeURIComponent(urlPincode);
            let encodedRoomType = encodeURIComponent(urlRoomType);

            let urlToBeCalled = urlPath + "SSID="+encodedSSID + "&Password="+encodedPassword + "&DeviceName="+encodedDeviceName + "&EmailID="+encodedEmailID + "&ReEnterEmailID="+encodedReEnterEmailID + "&Pincode="+encodedPincode + "&RoomType="+encodedRoomType;

            console.log("Url To Be Called: ", urlToBeCalled);

            let xhttp = new XMLHttpRequest();

            xhttp.onreadystatechange = function() {

            if (this.readyState == 4 && this.status == 200) {

                    if (this.responseText.includes("SUCCESS")) {
                        console.log("Response Received");
                        let arrResponse = this.responseText.split("|");
                        let clientBrowserUrl = arrResponse[1];

                        document.getElementById('loader').style.display = "block";
                        document.getElementById('blackfilm').style.display = "block";

                        setTimeout(processingRequestOne, 5000);
                        function processingRequestOne () {
                            console.log("Alert Received");

                            let confirmation = confirm("Device registration information has been received by the Device. Restarting Device. \nYou will now be redirected to HHM login page.");

                            document.getElementById('loader').style.display = "none";
                            document.getElementById('blackfilm').style.display = "none";

                            if(confirmation == true) {
                                console.log("Entered Confirmation Successfully");
                                document.getElementById('loader').style.display = "block";
                                document.getElementById('blackfilm').style.display = "block";

                                setTimeout(processingRequestTwo, 25000);
                                function processingRequestTwo () {
                                    // window.location.replace("http://localhost:3000/");
                                    window.location.replace(clientBrowserUrl);
                                }
                            } else {
                                console.log("Entry Denied");
                                document.getElementById('loader').style.display = "none";
                                document.getElementById('blackfilm').style.display = "none";
                            }  
                        }                      
                    } else if (this.responseText == "SPIFFS_ISSUE" ) {
                        alert("Registration failed.(Device experiencing issues)");
                    } else {
                        alert("Registration failed.");
                    }
                }
            };

            xhttp.open("PUT", urlToBeCalled, true);
            xhttp.send();

            // document.getElementById('loader').style.display = "block";
            // document.getElementById('blackfilm').style.display = "block";

        } else {
            if(Password.length < 8) {
                document.getElementById('invalidPassword').style.display = "block";
                document.getElementById('invalidPassword').innerText = "Password must be 8 characters.";
            }              
            if(DeviceName.length < 4) {
                document.getElementById('invalidDeviceName').style.display = "block";
                document.getElementById('invalidDeviceName').innerText = "Device Name must be 4 characters.";
            }
            if(validEmailRegex.test(EmailID) == false) {
                document.getElementById('invalidEmailID').style.display = "block";             
                document.getElementById('invalidEmailID').innerText = "Email ID is not valid."; 
            }
             if(ReEnterEmailID != EmailID) {
                document.getElementById('invalidReEnterEmailID').style.display = "block";
                document.getElementById('invalidReEnterEmailID').innerHTML = "Email ID does not match.";
            } 
        }    
    }

    function showHidePassword() {
        let showHidePasswordStatus = document.getElementById("Password");
        let showHidePasswordButtonText = document.getElementById("togglePasswordButton");

        if(showHidePasswordStatus.type == "password") {
            showHidePasswordStatus.type = "text";
            showHidePasswordButtonText.innerHTML = "HIDE";
        } else {
            showHidePasswordStatus.type = "password";
            showHidePasswordButtonText.innerHTML = "SHOW";            
        }
    }
    
</script>
)=====";