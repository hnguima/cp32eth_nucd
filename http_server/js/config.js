
var seconds = 10;
var theTimer;

document.getElementById("description").disabled = true;
console.log(document.querySelectorAll(`input[name^="enabled"]`))
document.querySelectorAll(`input[name^="enabled"]`).forEach( element => {
    checkEnabled(element)
})

var embedded_config;

// var embedded_config = JSON.parse('{\
//     "info": {\
//         "description": "Dispositivo: CP32ETH da NUCD",\
//         "fw_version": "0.0.1",\
//         "hw_version": "1.0",\
//         "resets": 1,\
//         "config_updates": 0,\
//         "if_updates": 0,\
//         "fw_updates": 0,\
//         "up_time": 0,\
//         "init_time": 0,\
//         "install_time": 0,\
//         "available_mem": 0\
//     },\
//     "login": {\
//         "username": "admin",\
//         "password": "ati12345"\
//     },\
//     "ip": {\
//         "ip": 167880896,\
//         "mask": 16777215,\
//         "gw": -33445696\
//     },\
//     "wifi": {\
//         "enabled": 1,\
//         "ssid": "CP32ETH",\
//         "password": "ati12345"\
//     },\
//     "socket": {\
//         "ip": 0,\
//         "port": 0\
//     }\
// }');

loadConfig()

getstatus(document.getElementById("config-update").querySelector(".file-drop-area"))
getstatus(document.getElementById("fw-update").querySelector(".file-drop-area"))

var changedConfigs = [];

let touchstartX = 0
let touchendX = 0

if( /Android|webOS|iPhone|iPad|iPod|BlackBerry/i.test(navigator.userAgent) ) {
    const slider = document.getElementById('slider')

    function handleGesture() {
        slider.style.left = `0px`
        if (touchendX - touchstartX > 100) swipe('right')
        else if (touchstartX - touchendX > 100) swipe('left')
    }

    function swipe(direction) {
        //select active element in sidebar
        sidebarItems = document.querySelector('.sidebar').querySelectorAll("a")
        sidebarItems.forEach(element => {
            let slideAmt = 0
            if (element.classList.contains('active')) {
                if(direction == "right"){
                        slideAmt= 100
                        sibling = element.previousElementSibling
                }
                else if(direction == "left"){
                        slideAmt= -100
                        sibling = element.nextElementSibling
                }

                
                if (Array.from(sidebarItems).find(node => node.isEqualNode(sibling))) {
                    slider.style.left = `${slideAmt}%`
                    setTimeout(() => {
                        sidebarChangeTab(sibling)
                        slider.style.transition = '0s'
                        slider.style.left = `${-slideAmt}%`
                    }, 50)	
                }
            }
        })

        setTimeout(() => {
            slider.style.transition = '0.2s'
            slider.style.left = `0px`
        }, 100)	
    }

    let movStarted = 0

    slider.addEventListener('touchstart', e => {
        touchstartX = e.changedTouches[0].screenX
    })

    slider.addEventListener('touchend', e => {
        touchendX = e.changedTouches[0].screenX
        handleGesture()
        movStarted = 0
    })


    slider.addEventListener('touchmove', e => {
        
        if(Math.abs(e.changedTouches[0].screenX - touchstartX) > 50 || movStarted){
            
            if(movStarted)
            slider.style.left = e.changedTouches[0].screenX - touchstartX + 'px'

            movStarted = 1
        }
    })
}

function inputChanged(input){
    
    var container = input.closest(".details-container");
    
    var index = changedConfigs.findIndex(item => item.id == container.id);
    
    if(index == -1){
        changedConfigs.push(container);
    }
    
    var button = document.getElementById("save-button");
    
    if(changedConfigs.length > 0){
        button.classList.remove("hidden");
        
        if(changedConfigs.length == 1){
            button.querySelector("span").innerText = 'Salvar ' + changedConfigs.length + ' alteração';
            
        } else {
            button.querySelector("span").innerText = 'Salvar ' + changedConfigs.length + ' alterações';
        }
    }

}

function checkEnabled(element) {
    var hidable = element.closest(".hidable")
    if(hidable == null){
        hidable = element.closest(".details-container")
    }
    
    hidable.setAttribute("disabled", element.checked ? "false" : "true")

    if(changedConfigs != null){
        filterInput(element)
    }
    return
}


function ReloadPage(){
    document.location.reload(true);
}

function sidebarChangeTab(tab){
    
    var sidebar = tab.closest(".sidebar");
    
    sidebar.querySelectorAll("a").forEach(item => {
        item.classList.remove("active");
        document.getElementById(item.id.split("-")[0]).classList.remove("active");
        
        if(item.id == tab.id){
            tab.classList.add("active");
            document.getElementById(tab.id.split("-")[0]).classList.add("active");
        }
    });
    
    filterInput(document.getElementById('description'));
    
}

async function loadConfig(){
    
    const response = await fetch("load_config", {
        method: "GET",
        headers: {"Content-type": "application/json", "Config-type" : "all" },
    });
    
    embedded_config = await response.json();

    console.log(embedded_config);
    
    for (const object in embedded_config) {
        var container = document.querySelector("#" + object);
        
        if (container){

            container.querySelectorAll("input, select, textarea, span").forEach(input => {

                
                if ((value = embedded_config[object][input.id]) != undefined ||
                (value = embedded_config[object][input.id.split("-")[0]]) != undefined) {
                    console.log(input.tagName + " " + value)
                    if(input.id.includes("ip-") || input.id.includes("mask-") || input.id.includes("gw-")){
                        input.value = value >> (input.id[input.id.length - 1] * 8) & 0xff
                    }
                    else if(input.type == "checkbox"){
                        input.checked = value
                        checkEnabled(input)
                    }
                    else if (input.tagName == "SPAN") { 
                        input.innerHTML += value
                    }
                    else {
                        input.value = value;
                    }
                }
            });
        }
    }
    
    filterInput(document.getElementById('description'));
    document.querySelectorAll("input[name=enabled]").forEach( element => {
        checkEnabled(element)
    })
    
    if(embedded_config.admin){
        document.querySelector("#admin").closest(".maindiv").classList.remove("hidden");
        document.querySelector("#interface-update").closest(".maindiv").classList.remove("hidden");
        document.getElementById("header-root").classList.remove("hidden");
        // document.querySelector("#admin").closest(".maindiv").classList.remove("hidden");
    }
    
    var file = new File([JSON.stringify(embedded_config, null, "\t")], "config.json");
    
    var configBackup = document.getElementById("config-backup"); 
    configBackup.href = window.URL.createObjectURL(file);
    configBackup.download = "config.json"
}

function filterInput(input){
    
    var container = input.closest(".details-container");
    var button = document.querySelector("#save-button");
    var popup = container.querySelector(".popup");
    popup.innerText = "";
    
    var isCorrect = true;
    
    try{
        
        container.querySelectorAll("input").forEach(input => {
            // ["ts", "tc", "timer"].some(item => {
            //     console.log(input.id)
            //     console.log(item)
            //     if(input.id.includes(item)){
            //     }
            // })

            if(input.parentElement.parentElement.getAttribute("disabled") != "true"){

                if(input.id.includes("ip") || input.id.includes("mask") || input.id.includes("gw") || 
                    input.id.includes("ts") || input.id.includes("tc") || input.id.includes("timer") ||
                    input.id.includes("port") || input.id.includes("slaveid")){
                    input.value = (input.value).replace(/[^0-9]/g,'');
                }
    
                if(input.id.includes("ip") || input.id.includes("mask") || input.id.includes("gw")){
                    if(input.value > 255){ input.value = 255;}
                }
                if(input.id.includes("ts")){
                    if(input.value > 31){ input.value = 31;}
                }
                if(input.id.includes("tc")){
                    if(input.value > 7){ input.value = 7;}
                }

                if(input.id.includes("ts") || input.id.includes("tc")){
                    var values = Array.from(container.querySelectorAll("input[id^='"+ input.id.substring(0, 2) +"-']"))
                                            .map(item => {if(item.value.length != 0) {return item.value}})
                                            .filter(item => item != undefined);
                    var set = new Set(values);
                    
                    if(values.length != set.size){
                        isCorrect = false;
                        popup.innerHTML = `Não podem haver teleobjetos repetidos`;
                        throw 'Break';
                    }
                }

                if(input.id.includes("ip") || input.id.includes("mask") || input.id.includes("gw") ||
                input.id.includes("ts") || input.id.includes("tc") || input.id.includes("timer") || input.id.includes("port")){
                    
                    if(input.value.length == 0){
                        isCorrect = false;
                        popup.innerHTML = `O campo "` + input.previousElementSibling.innerText + `" está vazio`
                        throw 'Break';
                    }
                }
                
                if(input.id.includes("password")){
                    
                    var passwords = container.querySelectorAll('input[type="password"]')
                    
                    if(input.value.length < 8 || input.value.length > 30){
                        isCorrect = false;
                        popup.innerHTML = "A senha precisa ter entre 8 e 30 caracteres"
                        throw 'Break';
                    }
                    else if(passwords[0].value != passwords[1].value){
                        isCorrect = false;
                        popup.innerHTML = "As senhas estão diferentes"
                        throw 'Break';
                    }                
                }

                if(input.id.includes("ssid") || input.id.includes("username")){
                    
                    if(input.value.length < 4 || input.value.length > 30){
                        isCorrect = false;
                        popup.innerHTML = `O ${input.id} precisa ter entre 4 e 30 caracteres`
                        throw 'Break';
                    }         
                }
            }
        });
    } catch (e) {
        if (e !== 'Break') throw e
    }
    
    container.querySelectorAll("textarea").forEach(textarea => {
        if(textarea.value.length > 300){
            textarea.value = textarea.value.substring(0,300)
        }
        
        textarea.style.height = "";
        textarea.style.height = textarea.scrollHeight + "px";
    });

    if(isCorrect){
        popup.setAttribute("active", "false");
        popup.innerHTML = ""
    }else{
        button.querySelector(".button-popup").innerHTML = popup.innerHTML;
        popup.setAttribute("active", "true");
    }

    var hasError = false;
    changedConfigs.forEach(config => {
        if(config.querySelector(".popup").getAttribute("active") == "true"){
            hasError = true;
        }
    });

    if(!hasError && isCorrect){
        button.classList.add("active")
    }else{
        button.classList.remove("active");
    }

    return !isCorrect
}

function revealFields(input, reveal){
    input.closest(".details-container").querySelectorAll("input")
    .forEach(password =>{
        if(password.id.includes("password")){
            password.type = reveal ? "text": "password";
        }
    })
    input.style.opacity = reveal ? "50%" : "100%";
}

async function sendConfig(form, body){
    
    if (filterInput(form)){ return }
    
    const formData = new FormData(form);
    const data = Object.fromEntries(formData.entries());

    console.log(data)
    
    for (const entry in data) {
        if(!isNaN(data[entry])) data[entry] = parseInt(data[entry]);
        
        if(entry == "ip" || entry == "mask" || entry == "gw"){
            data[entry] = formData.getAll(entry)[0] | formData.getAll(entry)[1] << 8 | formData.getAll(entry)[2] << 16 | formData.getAll(entry)[3] << 24
        }
    }

    body[form.id] = data;

    return true;
}

async function saveConfig(button){

    
    if(window.confirm("O dispositivo será reiniciado e as configurações serão salvas! ")){
        
        buttonContent = button.innerHTML;
        button.innerHTML = `<div class="save-button-load"></div>`;
        button.style.cursor = "not-allowed"

        var finished = false;
        var body = new Object;
        changedConfigs.forEach(async form => {
            finished = await sendConfig(form, body);
        });

        console.log(body);
        
        finished = true;
                
        const response = await fetch("save_config", {
            method: "PUT",
            headers: {"Content-type": "application/json", "Restart" : "true"}, 
            body: JSON.stringify(body)
        });
        
        var text = await response.text()

        // setTimeout(function () {
        //     location.reload();
        // }, 5000);
    }
}
        

function changeInfoConfig(button){
    
    textarea = button.closest(".details-container").querySelector("textarea")
    edit_btn = button.closest(".details-container").querySelector("#edit-info")
    save_btn = button.closest(".details-container").querySelector("#save-info")
    
    if (textarea.disabled) {
        save_btn.style.display = "block"
        edit_btn.style.display = "none"
        textarea.disabled = false;
    } else {
        save_btn.style.display = "none"
        edit_btn.style.display = "block"
        sendConfig(button.closest("form"))
        textarea.disabled = true;
    }
}

function loadFile(input) {
    var container = input.closest(".file-input-container");
    var file = input.files[0];
    
    console.log(container);
    
    container.querySelector(".file-msg").innerHTML = "Arquivo: " + file.name + "<br>" + "Tamanho: " + file.size + " bytes";
    container.querySelector(".button").style.display = "inline-block";
    
}
    
function loadFolder(input) {
    var container = input.closest(".file-input-container");

    var size = 0;
    
    for (let i=0; i< input.files.length; i++) {
        size = size + input.files[i].size;
    };

    container.querySelector(".file-msg").innerHTML = "Importados " + input.files.length + " arquivos" + "<br>" + "Tamanho total: " + size / 1000 + " Kb";
    container.querySelector(".button").style.display = "inline-block";
}

function updateFirmware(button){
    
    // Form Data
    var formData = new FormData();
    
    var container = button.closest(".file-input-container");
    
    var fileSelect = container.querySelector(".file-input");
    
    if (fileSelect.files && fileSelect.files.length == 1) {
        
        var file = fileSelect.files[0];
        
        formData.set("file", file, file.name);
        
        // Http Request
        var xhttp = new XMLHttpRequest();
        
        //Callback upload progress
        xhttp.upload.addEventListener("progress", function (evt) {
            
            if (evt.lengthComputable) {
                
                var percentComplete = (evt.loaded / evt.total) * 100;
                
                var x = Math.floor(percentComplete);
                
                //Do something with upload progress
                //console.log(x);
                
                container.querySelector(".file-status").innerHTML = "Progresso: " + x + "%.";
                
                // After we send it all, lets ask the board if it went well. 
                if (x == 100) {
                    // Lets ask the board for the upload resilt 
                    getstatus(button);
                }
                
            } else {
                window.alert('total size is unknown')
            }
        }, false);
        
        xhttp.open('POST', "/upload");
        xhttp.responseType = "blob";
        xhttp.send(formData);
        
    } else {
        
        window.alert('Selecione um arquivo primeiro')
    }
}


function getstatus(input) {
    
    var requestURL = "/status";
    
    var container = input.closest(".file-input-container");
    
    var xhttp = new XMLHttpRequest();
    // Call a function when the state changes.
    xhttp.onreadystatechange = function () { 
        if (this.readyState === XMLHttpRequest.DONE && this.status === 200) {
            var response = JSON.parse(xhttp.responseText);
            
            container.querySelector(".file-latest").innerHTML = "Último Firmware:  " + response.compile_date + " - " + response.compile_time;
            
            // If flashing was complete it will return a 1, else -1
            // A return of 0 is just for information on the Latest Firmware request
            if (response.status == 1) {
                // Start the countdown timer
                console.log("Status Request Returned ok");
                
                // Set the delay time in seconds 
                seconds = 10;
                
                // Start the countdown timer
                console.log("starting the timer");
                
                // Since ESP32 reported ok, lets set a tier to reload this web page 
                // Lets give it 10 seconds to reboot and re-connect to the router. 
                theTimer = setInterval(function () {
                    updateTimer(container.querySelector(".file-status"));
                }, 1000);
                
            } else if (response.status == -1) {
                container.querySelector(".file-status").innerHTML = "!!! Upload Error !!!";
            }
        }
    }
    console.log("Requestiing Upgrade Status");
    
    //Send the proper header information along with the request
    xhttp.open('POST', requestURL, true);
    xhttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    xhttp.send('status');
}

function updateTimer(status) {
    console.log("timer" + seconds);
    
    if (--seconds == 0) {
        
        clearInterval(theTimer);
        location.reload();
        
    } else {
        status.innerHTML = "Atualização completa. Reiniciar em " + seconds + " segundos.";
    }
}

function isValidJson(str) {
    try {
        JSON.parse(str);
    } catch (e) {
        return false;
    }
    return true;
}

async function updateConfig(button){
    
    // Form Data
    var formData = new FormData();

    var container = button.closest(".file-input-container");
    var fileSelect = container.querySelector(".file-input");
    
    if (fileSelect.files && fileSelect.files.length == 1) {

        var body = await fileSelect.files[0].text()
        
        if(isValidJson(await body)){
            const response = await fetch("save_config", {
                method: "PUT",
                headers: {"Content-type": "application/json", "Restart" : "true"}, 
                body: body
            });
            
            var text = await response.text()

            setTimeout(function () {
                location.reload();
            }, 5000);

        }else {
            
            window.alert('Arquivo JSON não é válido')
        }

        
    } else {
        
        window.alert('Selecione um arquivo primeiro')
    }
}

var prev_config = null;
async function prevConfigBackup(button){

    if(prev_config == null){
        try {
            const response = await fetch("load_config", {
                method: "GET",
                headers: {"Content-type": "application/json", "Config-type" : "old" },
            });

            prev_config = await response.json();

            var file = new File([JSON.stringify(prev_config, null, "\t")], "config_anterior.json");
            button.href = window.URL.createObjectURL(file);
            button.download = "config_anterior.json"

            button.click();

        } catch (e) {

            prev_config = null;
            window.alert("Configuração anterior não encontrada");
        }
        
    }
}

var factory_config = null;
async function factoryConfigBackup(button){

    if(factory_config == null){
        try {
            const response = await fetch("load_config", {
                method: "GET",
                headers: {"Content-type": "application/json", "Config-type" : "factory" },
            });

            factory_config = await response.json();

            var file = new File([JSON.stringify(factory_config, null, "\t")], "config_fabrica.json");
            button.href = window.URL.createObjectURL(file);
            button.download = "config_fabrica.json"

            button.click();

        } catch (e) {
            
            factory_config = null;
            window.alert("Configuração de fabrica não encontrada");
        }
        
    }
}

function sliceString(str, length) {
    var new_str = [];

    for(let i = 0; i < (str.length / length); i++){
        new_str[i] = str.slice(i * length, (i + 1) * length);
    }

    return new_str;
}

async function updateInterface(button){
    
    // Form Data
    var formData = new FormData();

    var container = button.closest(".file-input-container");
    var fileSelect = container.querySelector(".file-input");

            
    var folderName = fileSelect.files[0].webkitRelativePath.split("/")[0];
    
    if (fileSelect.files) {
        for (let file of fileSelect.files) {

            var body = await file.text();

            bodySlices = sliceString(body, 4096);

            var sliceIndex = 0;

            for(let slice of bodySlices){

                const response = await fetch( file.webkitRelativePath.replace(folderName + "/", "") , {
                    method: "PUT",
                    headers: {"Content-type": "application/octet-stream", "Slice-index" : `${sliceIndex}`}, 
                    body: slice
                });
                
                if(!(await response.ok)){
                    container.querySelector(".file-status").innerHTML = `Atualização falhou!`;
                    return;
                }
                
                sliceIndex = sliceIndex + 1;

                container.querySelector(".file-status").innerHTML = `Enviado: ${file.name} (${sliceIndex})`;
            }
        };

        setTimeout(function () {
            location.reload();
        }, 2000);

    } else {

        window.alert('Selecione uma pasta primeiro')
    }
}

async function resetDevice(isFactory){

    var doReset = false;

    if(isFactory){
        doReset = window.confirm(`Tem certeza que quer fazer o reset de fábrica? Todas as configurações internas serão DELETADAS. Ao aceitar será feito um backup no seu computador, permitindo que as configurações sejam revertidas, caso necessário. O dispositivo precisará ser configurado pelo wifi, já que as configurações de IP serão alteradas`)
    }else{
        doReset = window.confirm(`Tem certeza que quer fazer o reset?`)
    }

    if(!doReset){
        return;
    }

    const response = await fetch("reset", {
        method: "PUT",
        headers: {"Content-type": "application/json", "ResetType" : isFactory ? "factory" : "reset"},
    });

    setTimeout(function () {
        location.reload();
    }, 5000);
}
