import { v4 } from 'uuid';
import adjustLayoutModule from './adjustLayout';

export default function importModule(cy, nodeTypes) {
  const input = document.createElement('input');
  input.type = 'file';
  input.onchange = uploadedFile => {
    const { files } = uploadedFile.target;
    if (!files) return;
    const reader = new FileReader();
    reader.onload = file => {
      const nodes = JSON.parse(file.target.result);
      const map = [];
      const elements = [];

      nodes.forEach(element => {
        const elementItem = element;

        if (elementItem.group === 'nodes') {
          if (map[elementItem.data.id] === undefined) {
            map[elementItem.data.id] = v4();
            elementItem.data.id = map[elementItem.data.id];
            // elementItem.classes = elementItem.data.type;
            elements.push(
              nodeTypes.node({
                type: elementItem.data.type,
                data: elementItem.data,
              }),
            );
            elements.push(element);
          } else {
            throw new Error('Não pode continuar o processamento do arquivo!');
          }
        }
      });

      nodes.forEach(element => {
        const elementItem = element;

        if (elementItem.group === 'edges') {
          // console.log(elementItem, map[elementItem.data.id]);
          if (
            map[elementItem.data.id] !== undefined ||
            map[elementItem.data.source] === undefined ||
            map[elementItem.data.target] === undefined
          ) {
            throw new Error('Não pode continuar o processamento do arquivo!');
          } else {
            map[elementItem.data.id] = v4();
            elementItem.data.id = map[elementItem.data.id];
            elementItem.data.source = map[elementItem.data.source];
            elementItem.data.target = map[elementItem.data.target];
            // console.log(elementItem);
            elements.push(element);
          }
        }
      });
      // elements.forEach(e => console.log(e));
      cy.add(elements);
      adjustLayoutModule(cy);
    };
    reader.readAsText(files[0]);
  };
  input.click();
}
