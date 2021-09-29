// import { saveAs } from 'file-saver';

export default function exportModule(cy) {
  const json = cy.elements().jsons();
  // console.log(json);
  // console.log(JSON.stringify(json));
  return json;
  // const blob = new Blob([JSON.stringify(json)], {
  //   type: 'application/json;charset=utf-8',
  // });
  // saveAs(blob, 'circuit.json');
}
