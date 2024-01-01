$wallThickness = 1;
$wiggleRoom = 0.5;
$chamberDepth = 10;

$pcbMajorAxis = 45;
$pcbMinorAxis = 31;

$chevronHeight = 13;

$chamberLength = $pcbMajorAxis + 2*$wiggleRoom;
$chamberWidth = $pcbMinorAxis + 2*$wiggleRoom;
$baseLength = $chamberLength + 2*$wallThickness;
$baseWidth = $chamberWidth + 2*$wallThickness;

$clipWidth = 4;
$clipHeight = 2;

$fn=128;

module scale_width() {
    translate([$baseLength/2, $baseWidth/2, 0])
        scale([$baseLength/$baseWidth, 1, 1])
        children();
}

module chevron() {
    difference() {
        linear_extrude($chevronHeight) import("chevron.svg");
        translate([10, 40, 0]) cube([30, 30, 8]);
    }
}

module annulus(outer_radius, width, height) {
    linear_extrude(height) difference() {
        circle(outer_radius);
        circle(outer_radius - width);
    }
}

module base_top() {
    scale([1, 1, 5.5/$baseWidth])
        rotate_extrude() 
        intersection() {
           circle($baseWidth/2);
           square([$baseWidth, $baseWidth], center=false);
        }
    annulus($baseWidth/2, 1, 1.75);
}

module base() {
    scale_width()
    union() {
        cylinder($chamberDepth, d = $baseWidth);
        translate([0, 0, $chamberDepth]) base_top();
    }
}

module cavity_with_holes() {
    scale_width() cylinder($chamberDepth, d = $chamberWidth);
    translate([$baseLength/2, $baseWidth/2, $chamberDepth/2]) {
        translate([0, $baseWidth/2, 1]) cube([5, 5, 3], center=true);
        translate([$baseLength/2, 0, 0]) cube([5, 7, 3], center=true);
    }
    translate([$baseLength/2, 0, 0.5+$clipHeight/2]) {
        cube([$clipWidth, 5, 1], center=true);
        translate([0, $baseWidth, 0]) cube([$clipWidth, 5, 1], center=true);
    }
}

module combadge() {
    difference() {
        union() {
            color("#c0c0c0") translate([6, -2, 0]) scale([0.85, 0.85, 1]) chevron();
            color("#ffb700") base();
        }
        cavity_with_holes();
    }
}

module triangularPrism(side, depth, center=false) {
    linear_extrude(depth) polygon([[0, 0], [side, 0], [side/2, sqrt(3)*side/2]]);
}

module snapClip(height, width, center) {
    translate(center==true ? [-1, -width/2, 0] : [0, 0, 0])
    union() {
        translate([0, 0, height]) rotate([-90, 0, 0])
        linear_extrude(width)
        polygon([[0, 0], [1 ,0], [2, 1], [1, 1], [1, height], [0, height]]);
    }
}

module back() {
    translate([$baseLength/2, 0, $wallThickness]) union() {
        translate([0, 0.5+$wallThickness, 0]) rotate([0, 0, -90])
        snapClip($clipHeight, $clipWidth, true);
        translate([0, $baseWidth-(0.5+$wallThickness), 0]) rotate([0, 0, 90])
        snapClip($clipHeight, $clipWidth, true);
    }
    scale_width() color("#ffb700")
    cylinder($wallThickness, d=$baseWidth, center=false);
}

combadge();
translate([60, 0, 0])
back(); 