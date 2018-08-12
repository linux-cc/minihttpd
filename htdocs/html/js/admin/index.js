layui.define(['jquery', 'element', 'table'], function(exports) {
    var $ = layui.jquery,
        element = layui.element,
        bodyTab = 'bodyTab',
        title = $('.layui-tab-title'),
        showClass = 'layui-this';

    function navTreeClick() {
        $('.layui-nav-tree dd').click(function() {
            var layId = $(this).attr('lay-id');
            var src = (layId == 1 ? 'classes' : 'labels') + '.html';
            if (!isTabExist(layId)) {
                element.tabAdd(
                    bodyTab,
                    {
                        title: $(this).text() + '<i class="layui-icon layui-tab-close">&#x1006;</i>',
                        content: '<iframe src="' + src + '" style="width:100%; height:650px; border:0"></iframe>',
                        id: layId
                    }
                );
            }
            if (!isTabShow(layId)) {
                element.tabChange(bodyTab, layId);
            }
        });
    }

    function tabCloseClick() {
        title.on('click', '.layui-tab-close', function() {
            var li = $(this).parent();
            var layId = li.attr('lay-id');
            var preLayId = li.prev().attr('lay-id');
            if (isTabShow(layId)) {
                element.tabChange(bodyTab, preLayId);
            }
            element.tabDelete(bodyTab, layId);
        })
    }
    
    function isTabShow(layId) {
        return title.children(layIdFilter(layId)).hasClass(showClass);
    }
   
    function isTabExist(layId) {
        return title.children(layIdFilter(layId)).length;
    }


    function layIdFilter(layId) {
        return '[lay-id=' + layId + ']';
    }

    element.on('tab(' + bodyTab + ')', function(data) {
        if ($(this).is('li')) {
            var layId = $(this).attr('lay-id');
            $('.layui-nav-tree dd' + layIdFilter(layId)).click();
        }
    });
    navTreeClick();
    tabCloseClick();

    exports('index', {});
});

