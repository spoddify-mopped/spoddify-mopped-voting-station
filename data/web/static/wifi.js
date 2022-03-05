function success(image) {
    const div = document.createElement("div");
    div.classList.add("content")
    div.classList.add("success")

    div.appendChild(image);

    const headline = document.createElement("h2")
    headline.innerText = "Success";
    div.appendChild(headline);

    const span = document.createElement("span");
    span.innerText = "The Spoddify Mopped remote restarts now."

    div.appendChild(span);
    return div;
}

function formPost(path, event) {
    const image = new Image();
    image.className = "success";
    image.onload = function () {

        const request = new XMLHttpRequest();
        request.open('POST', path, true);

        request.onload = function () {
            document.body.replaceChildren(success(image))
        };

        request.onerror = function () {
        };

        request.send(event ? new FormData(event.target) : undefined);

    }
    image.src = "/static/success.png";
}

function updateWifi(event) {
    event.preventDefault();
    formPost("/wifi", event)
}

function restart() {
    if (!confirm("Do you want to restart?")) {
        return;
    }

    formPost("/restart")
}

function reset() {
    if (!confirm("This will reset all settings. Do you want to proceed?")) {
        return;
    }

    formPost("/reset")
}