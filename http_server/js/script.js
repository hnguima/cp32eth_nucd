
if(sessionStorage.getItem("isLogged") == null){
    sessionStorage.setItem("isLogged", 0);
}



function inputFilter(input) {
    input.value = (input.value).replace(/[^0-9]/g,'');
    if(parseInt(input.value, 10) > 255) input.value = 255;
}

function openNav() {
    document.getElementById("mySidebar").style.width = "250px";
}

function closeNav() {
    document.getElementById("mySidebar").style.width = "0";
}

async function sendHTTPReq(uri, body, element) {

    document.getElementById(element + "Ld").style.display = "inline-block";
    document.getElementById(element + "Txt").style.display = "none";
    
    const response = await fetch(uri, {
        method: "PUT",
        headers: {"Content-type": "application/json"}, 
        body: JSON.stringify(body)
    });
    var text = await response.text()
    console.log(text);

    if(!text.localeCompare("Success")){
        document.getElementById(element + "Chk").style.animation = "loadChk 1s";
        setTimeout(function(){
            document.getElementById(element + "Chk").style.animation = "idle 1s";
            document.getElementById(element + "Txt").style.display = "inline-block";
        }, 1000);
        document.getElementById(element + "Ld").style.display = "none";
    }

    return text;
}