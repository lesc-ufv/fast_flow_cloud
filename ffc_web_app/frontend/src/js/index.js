import '../css/style.scss';
import 'bootstrap';
import 'popper.js';
import 'bootstrap/scss/bootstrap.scss';

$('#new_project').click(() => {
  const htmlstring = `<form class="col">
  <div class="form-row">
    <div class="form-group col-md-10">
      <label for="id">Project Name</label><label for="id" style="color: red;">*</label>
      <input type="text" class="form-control" id="project-name" placeholder="">
    </div>
  </div>
  <div class="form-row">
    <div class="form-group col-md-10">
      <label for="sel-cgra">CGRA Architecture</label><label for="sel-cgra" style="color: red;">*</label>
      <select class="form-control" id="sel-cgra">
        <option value="">-- Select a CGRA --</option>
        <option value="1">CGRA 16 bits 128 PEs 8 Input Streams 8 Output Streams</option>
      </selected>
    </div>
  </div>
</form>`;
  $('.modal-dialog').removeClass('modal-lg');
  const modalBody = $('.modal-body');
  modalBody.children().remove();
  modalBody.html(htmlstring);
  $('#modalTitle').text('New Project');
  $('#btn-save').text('Create');
  $('#btn-close').text('Close');
  $('#modal-div').modal();
});

$('#btn-save').click(function(event){
  event.preventDefault();
  if($("#project-name").val() == ""){
    alert("The project name cannot be empty!");
  }else if($("#sel-cgra").val() == ""){
    alert("The CGRA architecture cannot be empty!");
  }else{
    location.href="project?project_name=" + $("#project-name").val();
  }
});
