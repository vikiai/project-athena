//
//  utils.js
//
//  Created by David Back on 19 Nov 2018
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

function disableDragDrop() {
    document.addEventListener("drop", function(event) {
        event.preventDefault();
        event.dataTransfer.effectAllowed = "none";
        event.dataTransfer.dropEffect = "none";
    }, false);
    
    document.addEventListener("dragover", function(event) {
        event.preventDefault();
        event.dataTransfer.effectAllowed = "none";
        event.dataTransfer.dropEffect = "none";
    }, false);
}
