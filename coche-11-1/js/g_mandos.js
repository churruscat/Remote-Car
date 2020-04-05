posVolante=0;
posAcel=0;
function enviaPost(datosJson) {
var url = "http://172.20.10.7/orden";
//			var url = "http://104.86.110.32";
   	request = new XMLHttpRequest();
   	request.onload = function () {
// Podiamos coger toda clase de info de la respuesta.
  		var status = request.status; // HTTP response status, e.g., 200 PARA "200 OK"
		var data = request.responseText; // DATOS DEVUELTOS, e.g., doc  HTML 
    }
request.open("POST", url, true);  //async=true
request.setRequestHeader("Content-Type", "text/plain;charset=UTF-8");
request.send(encodeURIComponent(datosJson));			
};

$(function($) {
  $(".volante").knob({
    change : function (value) {
    if (value==0){
        console.log("valor es cero: " + value);
      }
      valor=Math.round(value);
      if (valor!= posVolante) {
        posVolante=valor;
        enviaPost('orden:'+posVolante+','+posAcel);                       
      }
    },
    release : function (value) {
      enviaPost('orden:'+'0'+','+posAcel);
      if (value!=0){                           
         this.$.val('0');
      }
      posVolante=0;     
      this.val('0').trigger("change");
    },
    cancel : function () {
    },
    /*format : function (value) {
        return value + '%';
    },*/
    draw : function () {
        // Haz cosas de dibujar cada vez                      
    }
  });
});

$(function($) {
  $(".acelerador").knob({
    change : function (value) {
    if (value==0){
        console.log("valor es cero: " + value);
      }
      valor=Math.round(value);
      if (valor!= posAcel) {
        posAcel=valor;
        enviaPost('orden:'+posVolante+','+posAcel);                       
      }
    },
    release : function (value) {
        enviaPost('orden:'+posVolante+':'+'0');
        if (value!=0){                           
           this.$.val('0');
        }
        posAcel=0;     
        this.val('0').trigger("change");
    },
    cancel : function () {
    },
    /*format : function (value) {
        return value + '%';
    },*/
    draw : function () {
        // Haz cosas de dibujar cada vez                      
    }
  });
});      
