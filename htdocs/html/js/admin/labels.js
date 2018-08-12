layui.define(['jquery', 'element', 'form', 'table', 'layer'], function(exports) {

    initTable();
    openLayerBind('add', '添加标签', 'label-add.html');
    tableToolbarBind('labels');

    exports('labels', {});
});

function initTable() {
    renderTable('labels', [[
        {checkbox: true},
        {field: 'name', width: 400, title: '名称'},
        {field: 'father', width: 80, title: '状态'},
        {field: 'articles', width: 80, title: '文章数'},
        {fixed: 'right', width: 215, title: '操作', toolbar: '#labels_toolbar'},
    ]]);
}
