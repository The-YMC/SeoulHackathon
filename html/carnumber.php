<?php
    include('dbcon.php');
    include('check.php');

	$user_id = $_SESSION['user_id'];

	try { 
		$stmt = $con->prepare('select * from users where username=:username');
		$stmt->bindParam(':username', $user_id);
		$stmt->execute();

   } catch(PDOException $e) {
		die("Database error: " . $e->getMessage()); 
   }

    if (is_login()){
        if ($_SESSION['user_id'] == $user_id && $_SESSION['is_car']==0)
            ;
		else if ($_SESSION['user_id'] == $user_id && $_SESSION['is_car']==1)
            header("Location: carnumberdel.php");
    }else
        header("Location: index.php"); 
	include('head.php');
	
	if( ($_SERVER['REQUEST_METHOD'] == 'POST') && isset($_POST['submit']))
	{
 
        foreach ($_POST as $key => $val)
        {
            if(preg_match('#^__autocomplete_fix_#', $key) === 1){
                $n = substr($key, 19);
                if(isset($_POST[$n])) {
                    $_POST[$val] = $_POST[$n];
            }
        }
        } 

		$carnumber=$_POST['newcarnumber'];

		if(empty($carnumber)){
			$errMSG = "차량번호를 입력하세요.";
		}

			   try { 
                    $stmt = $con->prepare('select * from users where carnumber=:carnumber');
                    $stmt->bindParam(':carnumber', $carnumber);
                    $stmt->execute();

               } catch(PDOException $e) {
                    die("Database error: " . $e->getMessage()); 
               }

               $row = $stmt->fetch();
               if ($row){
                    $errMSG = "이미 등록된 차량입니다.";
               }



		if(!isset($errMSG))
		{
            try{
			$stmt = $con->prepare('UPDATE users SET carnumber=:carnumber, is_car=1 WHERE username=:username');
			$stmt->bindParam(':carnumber',$carnumber);
			$stmt->bindParam(':username',$user_id);
			
				if($stmt->execute())
				{
					$successMSG = "차량 번호 등록을 성공했습니다.";
				}
				else
				{
					$errMSG = "차량 등록 에러";
				}
                     } catch(PDOException $e) {
                        die("Database error: " . $e->getMessage()); 
                     }



		}


	}
?>

<head>
	<title>차량번호 등록</title>
	<link rel="stylesheet" href="bootstrap/css/bootstrap1.min.css">
</head>
<div class="container">
	<div>
	<h1 class="h2" align="center">&nbsp; <?php echo $user_id;  ?>의 차량번호 등록</h1><hr>
    </div>
	<?php
	if(isset($errMSG)){
			?>
            <div class="alert alert-danger">
            <span class="glyphicon glyphicon-info-sign"></span> <strong><?php echo $errMSG; ?></strong>
            </div>
            <?php
	}
	else if(isset($successMSG)){
		?>
        <div class="alert alert-success">
              <strong><span class="glyphicon glyphicon-info-sign"></span> <?php echo $successMSG; header('refresh:0');?></strong>
        </div>
        <?php		
		sleep(1);
		header("Location:logout.php");
	}
	?>   

	<form class="form-horizontal" method="POST">
		<div class="form-group" style="padding: 10px 10px 10px 10px;">
    <tr>
        <? $r1 = rmd5(rand().mocrotime(TRUE)); ?>
    	<p><td><label class="control-label">차량번호</label></td></p>
        <td><input class="form-control" type="text" name="<? echo $r1; ?>" placeholder="빌리신 차량의 번호를 입력하세요." autocomplete="off" readonly 
    onfocus="this.removeAttribute('readonly');" />
            <input type="hidden" name="__autocomplete_fix_<? echo $r1; ?>" value="newcarnumber" /> 

        </td>
		<br>
    </tr>
    <tr>
        <td colspan="2" align="center">
        <button type="submit" name="submit"  class="btn btn-primary"><span class="glyphicon glyphicon-floppy-save"></span>&nbsp; 등록</button>
        </td>
    </tr>
    </table>
</form>
</div>
</body>