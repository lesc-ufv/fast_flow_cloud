import { v4 } from 'uuid';

export function removeElements(elements) {
  // console.log(elements.parent());
  const parentNodes = elements.parent();
  elements.remove();

  parentNodes.forEach(parent => {
    if (!parent.isParent()) removeElements(parent);
    else if (parent.children().length === 1) ungroup(parent);
  });
  // return (
  //   cy.remove(elements).length &&
  //   removeElements(
  //     cy.$('.parent').filter(node => !node.children().length),
  //     cy,
  //   )
  // );
}

export function group(nodes, cy) {
  const parentNode = cy.add({
    group: 'nodes',
    data: {
      id: v4(),
    },
    classes: ['parent'],
  });

  // console.log(nodes.data());
  // const childNodes = elements.filter('node');
  // console.log(parentNode.data());
  nodes.forEach(node => {
    node.move({
      parent: parentNode.id(),
    });
  });
  // console.log('data', childNodes.data());
  // console.log(childNodes.parent().data());
}

export function ungroup(parent) {
  const grandpaId = parent.isChild() ? parent.parent().id() : null;

  parent.children().move({
    parent: grandpaId,
  });
  removeElements(parent); // eslint-disable-line
}

export function subgroup(nodes, cy) {
  const parentNode = cy.add({
    group: 'nodes',
    data: {
      id: v4(),
    },
    classes: ['parent'],
  });

  parentNode.move({ parent: nodes[0].parent().id() });

  nodes.forEach(node => {
    node.move({
      parent: parentNode.id(),
    });
  });
}

export function insertNode(node, parent, cy) {
  node.move({
    parent: parent.id(),
  });
}

export function merge(parent1, parent2, cy) {}

export function removeFromGroup(node) {
  const parent = node.parent();
  const grandpaId = parent.isChild() ? parent.parent().id() : null;

  node.move({
    parent: grandpaId,
  });

  if (!parent.isParent()) removeElements(parent);
  else if (parent.children().length === 1) ungroup(parent);
}
